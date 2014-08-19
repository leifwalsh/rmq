/**
 * Defines the interface we'll use to check each of our RMQ
 * implementations.
 */

#pragma once

#include <iterator>

template<
  typename iterator_type,
  typename value_type=typename std::iterator_traits<iterator_type>::value_type,
  typename difference_type=typename std::iterator_traits<iterator_type>::difference_type
  >
class rmq {

  /**
   * Beginning of the input array (so we can return an iterator into the
   * original array, rather than just an index).
   */
  const iterator_type _begin;

  /**
   * Endof the input array.
   */
  const iterator_type _end;

protected:

  iterator_type begin() const { return _begin; }

  iterator_type end() const { return _end; }

  difference_type n() const { return _end - _begin; }

  const value_type &val(difference_type i) const { return _begin[i]; }

public:
  rmq() = delete;

  /**
   * Preprocess the array [b,e) for RMQ queries.
   */
  rmq(iterator_type b, iterator_type e)
    : _begin(b),
      _end(e)
  {}

  /**
   * Query for the index of the minimum value between u and v.
   *
   * Preconditions:
   *  b <= u <= v <= e
   */
  virtual difference_type query(iterator_type u, iterator_type v) const = 0;

  difference_type query_offset(difference_type uo, difference_type vo) const {
    return query(begin() + uo, begin() + vo);
  }
};
