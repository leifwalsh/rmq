/**
 * Implements the sparse <O(n log n), O(1)> RMQ solution.
 */

#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>

#include <boost/iterator/counting_iterator.hpp>

#include "rmq.hpp"

template<
  typename iterator_type,
  typename value_type=typename std::iterator_traits<iterator_type>::value_type,
  typename difference_type=typename std::iterator_traits<iterator_type>::difference_type
  >
class sparse_rmq : public rmq<iterator_type, value_type, difference_type> {

  // Compilers are dumb.
  using rmq<iterator_type, value_type, difference_type>::begin;
  using rmq<iterator_type, value_type, difference_type>::end;
  using rmq<iterator_type, value_type, difference_type>::n;
  using rmq<iterator_type, value_type, difference_type>::val;

  typedef typename std::vector<std::vector<difference_type>>::difference_type level_type;

  /**
   * The log_2 of the problem size, lg(n) (this is the depth we need to
   * precompute answers down to).
   */
  const level_type _logn;

  /**
   * 2D array of precomputed answers.
   *
   * _arr[a][b] is the minimum value in the range [a, a+b].
   */
  std::vector<std::vector<difference_type>> _arr;

  /**
   * Dynamic program to fill in _arr.
   */
  void fill_in() {
    // Each interval of zero length starting at i should return the value
    // at i.
    std::copy_n(boost::counting_iterator<difference_type>(0), n(),
                std::back_inserter(_arr[0]));
    
    // The depth goes up to lg(n).
    for (level_type d = 0; d < _logn; ++d) {
      // We need to compute all intervals of length 2^d up through the one
      // that starts 2^d away from the end, hence the upper limit here is
      // (n - 2^d).
      const typename std::vector<difference_type>::difference_type width = 1<<d;
      // Form the next array by zipping pairs of elements in the dth array
      // that are width apart, taking the index of the lesser one.
      std::transform(_arr[d].begin(), _arr[d].end() - width,
                     _arr[d].begin() + width,
                     std::back_inserter(_arr[d+1]),
                     [this](const difference_type &x, const difference_type &y) {
                       return val(x) < val(y) ? x : y;
                     });
    }
  }

public:
  sparse_rmq(iterator_type b, iterator_type e)
    : rmq<iterator_type, value_type, difference_type>(b, e),
      _logn(std::max(difference_type(1),
                     difference_type(std::floor(std::log2(n()))))),
      _arr(_logn + 1)
  {
    std::fill(_arr.begin(), _arr.end(),
              std::vector<difference_type>());

    fill_in();
  }

  difference_type query(iterator_type u, iterator_type v) const {
    const level_type depth = std::max(level_type(0),
                                      level_type(std::floor(std::log2(v-u-1))));
    const iterator_type &b = begin();
    const auto x = u-b;
    const auto y = v-b;
    const difference_type px = _arr[depth][x];
    const difference_type py = _arr[depth][y-(1<<depth)];
    return val(px) < val(py) ? px : py;
  }
};
