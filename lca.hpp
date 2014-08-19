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
   * Input tree structure.
   */
  const tree<value_type> &_input;

  /**
   * The Euler tour of the input.
   */
  std::vector<value_type> _euler;

  /**
   * A vector of the same length as the Euler tour, stores the level of
   * the node from which the ith element of the Euler tour came.
   */
  std::vector<ssize_t> _level;

  /**
   * The ±1 RMQ data structure for the _level array.
   */
  std::unique_ptr<rmq_impl> _rmq;

  template<
    typename OutputIterator1,
    typename OutputIterator2
    >
  void preprocess(const tree<value_type> &t,
                  ssize_t level,
                  OutputIterator1 eulerit,
                  OutputIterator2 levelit) {
    // Constructs an Euler tour by running a DFS on the tree, emitting the
    // root of each subtree upon arrival and also upon completion of the
    // search of each of its children.
    auto val = t.id();
    *eulerit++ = val;
    *levelit++ = level;

    // In the paper, we use a "representative array" R, which maps node
    // ids to an index in the Euler tour.  Since our node ids may not be
    // consecutive integers, in order to get O(1) access to the
    // representative for a node, we must store it in the tree node
    // itself.
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
    // After preprocessing, all nodes in the tree should have their repr()
    // initialized.  We use that to find the indexes on which to run the
    // RMQ algorithm.
    auto ui = u.repr();
    auto vi = v.repr();

    // The RMQ interface uses an exclusive upper bound so we need to go
    // one past that to include the node represented by the upper bound
    // here.
    auto idx = (ui <= vi
                ? _rmq->query(_level.begin() + ui, _level.begin() + vi + 1)
                : _rmq->query(_level.begin() + vi, _level.begin() + ui + 1));

    // Future work: return the node itself, rather than the node's id?
    return _euler[idx];
  }
};

