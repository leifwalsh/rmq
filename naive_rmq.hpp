/**
 * Implements the naive <O(n^2), O(1)> RMQ solution.
 */

#include <algorithm>
#include <iterator>
#include <vector>

#include <boost/iterator/counting_iterator.hpp>

#include "rmq.hpp"

template<
  typename iterator_type,
  typename value_type=typename std::iterator_traits<iterator_type>::value_type,
  typename difference_type=typename std::iterator_traits<iterator_type>::difference_type
  >
class naive_rmq : public rmq<iterator_type, value_type, difference_type> {

  // Compilers are dumb.
  using rmq<iterator_type, value_type, difference_type>::begin;
  using rmq<iterator_type, value_type, difference_type>::end;
  using rmq<iterator_type, value_type, difference_type>::n;
  using rmq<iterator_type, value_type, difference_type>::val;

  /**
   * 2D array of precomputed answers.
   *
   * _arr[b][a] is the index of the minimum value in the range [a, a+b+1].
   */
  std::vector<std::vector<difference_type>> _arr;

  /**
   * Dynamic program to compute the answers to every possible query on the
   * input.
   */
  void fill_in() {
    // The first level _arr[0] contains the answers to RMQ queries on
    // intervals of length 1, which must just be the first element in the
    // interval.
    std::copy_n(boost::counting_iterator<difference_type>(0), n(),
                std::back_inserter(_arr[0]));

    // Fill in each consecutive level by choosing the smaller of each
    // neighboring values in the previous level.
    for (auto it = _arr.begin(); it + 1 < _arr.end(); ++it) {
      // Zips neighboring indexes of this level together with a functor
      // that chooses the index producing the lesser value, from each pair
      // of indexes.
      std::transform(it->begin(), it->end() - 1,
                     it->begin() + 1,
                     std::back_inserter(*(it + 1)),
                     [this](const difference_type &x, const difference_type &y) {
                       return val(x) < val(y) ? x : y;
                     });
    }
  }

public:
  naive_rmq(iterator_type b, iterator_type e)
    : rmq<iterator_type, value_type, difference_type>(b, e),
      _arr(n())
  {
    std::fill(_arr.begin(), _arr.end(),
              std::vector<difference_type>());

    fill_in();
  }

  difference_type query(iterator_type u, iterator_type v) const {
    return _arr[v-u-1][u-begin()];
  }
};
