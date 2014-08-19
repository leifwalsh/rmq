/**
 * Implements the <O(n), O(1)> ±1 RMQ solution.
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/tuple/tuple.hpp>

#include "rmq.hpp"

#include "naive_rmq.hpp"
#include "sparse_rmq.hpp"

template<
  typename iterator_type,
  typename value_type=typename std::iterator_traits<iterator_type>::value_type,
  typename difference_type=typename std::iterator_traits<iterator_type>::difference_type
  >
class pm_rmq : public rmq<iterator_type, value_type, difference_type> {

  // Compilers are dumb.
  using rmq<iterator_type, value_type, difference_type>::begin;
  using rmq<iterator_type, value_type, difference_type>::end;
  using rmq<iterator_type, value_type, difference_type>::n;
  using rmq<iterator_type, value_type, difference_type>::val;

  /**
   * The log_2 of the problem size, lg(n).
   */
  const difference_type _logn;

  /**
   * The block size is lg(n)/2.
   */
  const difference_type block_size() const {
    return std::max(difference_type(1), _logn / 2);
  }

  /**
   * Arrays of length 2n/lg(n) where the first array contains the minimum
   * element in the ith block of the input, and the second contains the
   * position of that element (as an offset from the beginning of the
   * original input, not from the beginning of the block).
   */
  std::vector<value_type> _super_array_vals;
  std::vector<difference_type> _super_array_idxs;

  /**
   * The sparse RMQ implementation over _super_array_vals;
   */
  std::unique_ptr<sparse_rmq<typename std::vector<value_type>::const_iterator> > _super_rmq;

  /**
   * We use a vector<value_type> to represent a normalized block.  We
   * could use a vector<bool> but that would require building another
   * temporary array, this is simpler, if less space efficient.
   *
   * We keep a map of them while building them to dedupe and to own the
   * memory, and an array of them indexed by block number to use at query
   * time.
   *
   * This is the biggest opportunity for optimization.  The paper suggests
   * keeping a table of all sqrt(n) possible sub blocks.  We only keep
   * ones that exist in order to simplify handling of the last block
   * (which might be shorter).  However, our solution involves a bunch of
   * copying that makes it slower.
   */
  typedef std::vector<value_type> block_identifier;

  /**
   * We'll use a naive_rmq over block_identifiers to represent the
   * precomputed RMQ answers for sub blocks.
   */
  typedef naive_rmq<typename std::vector<value_type>::const_iterator> sub_block_rmq;

  /**
   * The map from block_identifier to the naive_rmq solution on blocks of
   * that shape.  This map is responsible for the lifetime of each
   * naive_rmq, and for ensuring that we only build one naive_rmq for each
   * shape of sub block.  Once the naive_rmq structures are built, we
   * don't really use this map again.
   */
  std::map<block_identifier, std::unique_ptr<sub_block_rmq> > _sub_block_rmqs;

  /**
   * An array mapping sub blocks (by their index) to the naive_rmq
   * implementation representing sub blocks of their shape.  We need to do
   * this lookup at query time so we can't afford to reconstruct the
   * block_identifier and query in the map.
   */
  std::vector<sub_block_rmq *> _sub_block_rmq_array;

  /**
   * Normalizes a sub_block so that it starts with 0.  Appends results to
   * an OutputIterator.
   */
  template<
    typename OutputIterator
    >
  static void normalize(iterator_type b, iterator_type e,
                        OutputIterator o) {
    const value_type &init = *b;
    std::transform(b, e, o,
                   [init](const value_type &val) { return val - init; });
  }

public:
  pm_rmq(iterator_type b, iterator_type e)
    : rmq<iterator_type, value_type, difference_type>(b, e),
      _logn(std::max(difference_type(1),
                     difference_type(std::floor(std::log2(n())))))
  {
#ifndef NDEBUG
    // Check the ±1 property.
    std::for_each(boost::make_zip_iterator(boost::make_tuple(b, b + 1)),
                  boost::make_zip_iterator(boost::make_tuple(e - 1, e)),
                  [b, e](const boost::tuple<const value_type&, const value_type&>& t) {
                    auto diff = t.template get<0>() - t.template get<1>();
                    assert(diff == -1 || diff == 1);
                  });
#endif

    // For each sub_block, we'll add it to the _super_arrays and also
    // normalize it and compute its naive_rmq.
    for (iterator_type block_begin = begin(); block_begin < end(); block_begin += block_size()) {
      const iterator_type block_end = std::min(block_begin + block_size(), end());

      // Find the min element of the block by brute force.
      const iterator_type block_min = std::min_element(block_begin, block_end);
      _super_array_vals.push_back(*block_min);
      _super_array_idxs.push_back(block_min - begin());

      // Compute the normalized block.
      std::vector<value_type> normalized_block(block_end - block_begin);
      normalize(block_begin, block_end, normalized_block.begin());

      // Find the normalized block in the map of RMQ structures.  If not
      // found, construct one.
      std::unique_ptr<sub_block_rmq> &naive_ptr = _sub_block_rmqs[normalized_block];
      if (!naive_ptr) {
        naive_ptr.reset(new sub_block_rmq(normalized_block.begin(), normalized_block.end()));
      }

      // Either way, record a pointer to the RMQ structure at this
      // sub_block's index.
      _sub_block_rmq_array.push_back(naive_ptr.get());
    }

    // Construct the RMQ structure over the super array.
    _super_rmq.reset(new sparse_rmq<typename std::vector<value_type>::const_iterator>(_super_array_vals.begin(), _super_array_vals.end()));
  }

  difference_type query(iterator_type u, iterator_type v) const {
    // The overall strategy here is straight from the paper.  We look up
    // the blocks that contain u and v (taking care to consider v being
    // the inclusive endpoint even though the API understands it to be
    // exclusive), then use a sparse_rmq search over the super array among
    // blocks strictly between u's and v's blocks, and do naive_rmq
    // searches in the representative structures for the blocks with u's
    // and v's blocks' shapes.
    //
    // Most of what's below is dealing with types and offset math, and
    // isn't all that interesting.

    const difference_type u_block_idx = difference_type(u - begin()) / block_size();
    const difference_type u_offset = difference_type(u - begin()) % block_size();
    const difference_type v_block_idx = difference_type(v - 1 - begin()) / block_size();
    const difference_type v_offset = difference_type(v - 1 - begin()) % block_size();

    const sub_block_rmq &u_naive = *_sub_block_rmq_array[u_block_idx];
    const sub_block_rmq &v_naive = *_sub_block_rmq_array[v_block_idx];

    const difference_type block_diff = v_block_idx - u_block_idx;
    if (block_diff == 0) {

      // u and v are in the same block.  One naive_rmq search suffices.
      return (u_block_idx * block_size()) + u_naive.query_offset(u_offset, v_offset + 1);

    } else {

      const iterator_type u_block_end = std::min(end(), begin() + ((u_block_idx + 1) * block_size()));

      // u and v are in different blocks.  First, do naive_rmq searches in
      // each block from u to the end of its block, and from the beginning
      // of v's block to v.  Then also translate these to offsets within
      // the original array, because that's what we intend to return.
      const difference_type u_min_idx = (u_block_idx * block_size()) +
        u_naive.query_offset(u_offset,
                             u_block_end - (begin() + (block_size() * u_block_idx)));
      const difference_type v_min_idx = (v_block_idx * block_size()) +
        v_naive.query_offset(0, v_offset + 1);

      if (block_diff == 1) {

        // u and v are in adjacent blocks.  Don't query the super array,
        // it doesn't handle zero-length intervals properly.
        return val(u_min_idx) < val(v_min_idx) ? u_min_idx : v_min_idx;

      } else {

        // Full algorithm, using the sparse RMQ implementation on the
        // super array between u's and v's blocks.

        const difference_type super_idx = _super_rmq->query(_super_array_vals.begin() + u_block_idx + 1,
                                                            _super_array_vals.begin() + v_block_idx);

        const value_type &u_min_val = val(u_min_idx);
        const value_type &v_min_val = val(v_min_idx);
        if (u_min_val < v_min_val) {
          return u_min_val < _super_array_vals[super_idx] ? u_min_idx : _super_array_idxs[super_idx];
        } else {
          return v_min_val < _super_array_vals[super_idx] ? v_min_idx : _super_array_idxs[super_idx];
        }
      }
    }
  }
};
