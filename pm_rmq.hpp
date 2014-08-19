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

  const difference_type block_size() const {
    return std::max(difference_type(1), _logn / 2);
  }

  /**
   * Arrays of length 2n/lg(n) where the first array contains the minimum
   * element in the ith block of the input, and the second contains the
   * position of that element.
   */
  std::vector<value_type> _super_array_vals;
  std::vector<difference_type> _super_array_idxs;

  /**
   * The sparse RMQ implementation over _super_array;
   */
  std::unique_ptr<sparse_rmq<typename std::vector<value_type>::const_iterator>> _super_rmq;

  /**
   * We use a vector<value_type> to represent a normalized block.  We
   * could use a vector<bool> but that would require building another
   * temporary array, this is simpler, if less space efficient.
   *
   * We keep a map of them while building them to dedupe and to own the
   * memory, and an array of them indexed by block number to use at query
   * time.
   */
  typedef std::vector<value_type> block_identifier;
  std::map<block_identifier, std::unique_ptr<naive_rmq<typename std::vector<value_type>::const_iterator>>> _sub_block_rmqs;
  std::vector<naive_rmq<typename std::vector<value_type>::const_iterator> *> _sub_block_rmq_array;

  static std::vector<value_type> normalize(iterator_type b, iterator_type e) {
    std::vector<value_type> normalized_block;
    const value_type &init = *b;
    std::transform(b, e, std::back_inserter(normalized_block),
                   [init](const value_type &val) { return val - init; });
    return normalized_block;
  }

public:
  pm_rmq(iterator_type b, iterator_type e)
    : rmq<iterator_type, value_type, difference_type>(b, e),
      _logn(std::max(difference_type(1),
                     difference_type(std::floor(std::log2(n())))))
  {
    // Check the ±1 property.
    std::for_each(boost::make_zip_iterator(boost::make_tuple(b, b + 1)),
                  boost::make_zip_iterator(boost::make_tuple(e - 1, e)),
                  [b, e](const boost::tuple<const value_type&, const value_type&>& t) {
                    auto diff = t.template get<0>() - t.template get<1>();
                    assert(diff >= -1 && diff <= 1);
                  });

    for (iterator_type block_begin = begin(); block_begin < end(); block_begin += block_size()) {
      _super_array_vals.push_back(*block_begin);
      _super_array_idxs.push_back(block_begin - begin());
      for (iterator_type cur = block_begin + 1; cur != block_begin + block_size() && cur != end(); ++cur) {
        if (*cur < _super_array_vals.back()) {
          _super_array_vals.back() = *cur;
          _super_array_idxs.back() = cur - begin();
        }
      }

      const std::vector<value_type> normalized_block = normalize(block_begin, std::min(block_begin + block_size(), end()));
      std::unique_ptr<naive_rmq<typename std::vector<value_type>::const_iterator>> &naive_ptr = _sub_block_rmqs[normalized_block];
      if (!naive_ptr) {
        naive_ptr.reset(new naive_rmq<typename std::vector<value_type>::const_iterator>(normalized_block.begin(), normalized_block.end()));
      }
      _sub_block_rmq_array.push_back(naive_ptr.get());
    }

    _super_rmq.reset(new sparse_rmq<typename std::vector<value_type>::const_iterator>(_super_array_vals.begin(), _super_array_vals.end()));
  }

  difference_type query(iterator_type u, iterator_type v) const {
    const typename std::vector<value_type>::difference_type u_block_idx = difference_type(u - begin()) / block_size();
    const typename std::vector<value_type>::difference_type u_offset = difference_type(u - begin()) % block_size();
    const typename std::vector<value_type>::difference_type v_block_idx = difference_type(v - 1 - begin()) / block_size();
    const typename std::vector<value_type>::difference_type v_offset = difference_type(v - 1 - begin()) % block_size();

    const typename std::vector<value_type>::difference_type block_diff = v_block_idx - u_block_idx;
    if (block_diff == 0) {
      // u and v are in the same block.
      auto naive = *_sub_block_rmq_array[u_block_idx];
      return (u_block_idx * block_size()) + naive.query_offset(u_offset, v_offset + 1);
    } else if (block_diff == 1) {
      // u and v are in adjacent blocks.
      auto u_naive = *_sub_block_rmq_array[u_block_idx];
      auto u_min_idx = (u_block_idx * block_size()) +
        u_naive.query_offset(u_offset,
                             std::min(end(), begin() + ((u_block_idx + 1) * block_size())) - (begin() + (block_size() * u_block_idx)));
      auto v_naive = *_sub_block_rmq_array[v_block_idx];
      auto v_min_idx = (v_block_idx * block_size()) + v_naive.query_offset(0, v_offset + 1);
      return val(u_min_idx) < val(v_min_idx) ? u_min_idx : v_min_idx;
    } else if (block_diff == 2) {
      // u and v have one block in between.
      auto u_naive = *_sub_block_rmq_array[u_block_idx];
      auto u_min_idx = (u_block_idx * block_size()) +
        u_naive.query_offset(u_offset,
                             std::min(end(), begin() + ((u_block_idx + 1) * block_size())) - (begin() + (block_size() * u_block_idx)));
      auto v_naive = *_sub_block_rmq_array[v_block_idx];
      auto v_min_idx = (v_block_idx * block_size()) + v_naive.query_offset(0, v_offset + 1);
      if (val(u_min_idx) < val(v_min_idx)) {
        return val(u_min_idx) < _super_array_vals[u_block_idx + 1] ? u_min_idx : _super_array_idxs[u_block_idx + 1];
      } else {
        return val(v_min_idx) < _super_array_vals[u_block_idx + 1] ? v_min_idx : _super_array_idxs[u_block_idx + 1];
      }
    } else {
      // Full algorithm, using the sparse RMQ implementation between u's and v's blocks.
      const auto uend = std::min(end(),
                                 begin() + (block_size() * (u_block_idx + 1)));
      sparse_rmq<iterator_type> cheater1(u, uend);
      const difference_type first_block_min = cheater1.query(u, uend);
      const auto vbegin = begin() + (block_size() * v_block_idx);
      sparse_rmq<iterator_type> cheater2(vbegin, v);
      const difference_type last_block_min = cheater2.query(vbegin, v);
      const typename std::vector<value_type>::difference_type super_idx = _super_rmq->query(_super_array_vals.begin() + u_block_idx + 1,
                                                                                             _super_array_vals.begin() + v_block_idx);
      if (*(u + first_block_min) < _super_array_vals[super_idx]) {
        return *(u + first_block_min) < *(vbegin + last_block_min) ? (u + first_block_min - begin()) : (vbegin + last_block_min - begin());
      } else {
        return *(vbegin + last_block_min) < _super_array_vals[super_idx] ? (vbegin + last_block_min - begin()) :
          _super_array_idxs[super_idx];
      }
    }
  }
};
