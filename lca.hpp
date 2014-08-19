/**
 * Solves the LCA problem by running the ±1 RMQ solution (or any RMQ
 * implementation, actually) on the Euler tour of the tree.
 */

#include <algorithm>
#include <memory>
#include <vector>

#include "tree.hpp"

template<
  typename value_type,
  typename rmq_impl
  >
class lca {

  /**
   * Beginning of the input array (so we can return an iterator into the
   * original array, rather than just an index).
   */
  const tree<value_type> &_input;

  std::vector<value_type> _euler;
  std::vector<ssize_t> _level;
  std::unique_ptr<rmq_impl> _rmq;

  template<
    typename OutputIterator1,
    typename OutputIterator2
    >
  void preprocess(const tree<value_type> &t,
                  ssize_t level,
                  OutputIterator1 eulerit,
                  OutputIterator2 levelit) {
    auto val = t.id();
    *eulerit++ = val;
    *levelit++ = level;
    const_cast<tree<value_type> &>(t).set_repr(_level.end() - 1 - _level.begin());
    std::for_each(t.children().begin(), t.children().end(),
                  [this, val, level, &eulerit, &levelit](const tree<value_type> &c) {
                    preprocess(c, level + 1, eulerit, levelit);
                    *eulerit++ = val;
                    *levelit++ = level;
                  });
  }

  void preprocess() {
    preprocess(_input, 0, std::back_inserter(_euler), std::back_inserter(_level));
    _rmq.reset(new rmq_impl(_level.begin(), _level.end()));
  }

public:
  lca(const tree<value_type> &t)
    : _input(t)
  {
    preprocess();
  }

  value_type query(const tree<value_type> &u, const tree<value_type> &v) const {
    auto ui = u.repr();
    auto vi = v.repr();
    auto idx = (ui <= vi
                ? _rmq->query(_level.begin() + ui, _level.begin() + vi + 1)
                : _rmq->query(_level.begin() + vi, _level.begin() + ui + 1));
    return _euler[idx];
  }
};

