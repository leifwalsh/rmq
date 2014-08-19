/**
 * Implements the optimal <O(n), O(1)> RMQ solution by converting to LCA
 * and then back to ±1 RMQ.
 */

#include <algorithm>
#include <assert.h>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include <boost/tuple/tuple.hpp>

#include "lca.hpp"
#include "pm_rmq.hpp"
#include "tree.hpp"

template<
  typename iterator_type,
  typename value_type=typename std::iterator_traits<iterator_type>::value_type,
  typename difference_type=typename std::iterator_traits<iterator_type>::difference_type
  >
class opt_rmq : public rmq<iterator_type, value_type, difference_type> {

  // Compilers are dumb.
  using rmq<iterator_type, value_type, difference_type>::begin;
  using rmq<iterator_type, value_type, difference_type>::end;
  using rmq<iterator_type, value_type, difference_type>::n;
  using rmq<iterator_type, value_type, difference_type>::val;

  typedef std::pair<value_type, difference_type> tree_val_type;
  typedef tree<tree_val_type> tree_type;

  /**
   * Array associating indexes to tree nodes.
   */
  std::vector<const tree_type *> _idx_to_node;

  /**
   * Cartesian tree of the input array.
   */
  tree_type _tree;

  lca<tree_val_type, pm_rmq<std::vector<ssize_t>::const_iterator> > _lca;

  static tree_type cartesian_tree(iterator_type b, iterator_type e) {
    tree_type root(tree_val_type(*b, 0));
    std::vector<tree_type *> rightmost_path;
    rightmost_path.push_back(&root);
    for (iterator_type c = b + 1; c != e; ++c) {
      while (!rightmost_path.empty() && rightmost_path.back()->id().first > *c) {
        rightmost_path.pop_back();
      }
      tree_val_type tree_val(*c, c - b);
      if (rightmost_path.empty()) {
        {
          std::vector<tree_type> new_roots_children;
          new_roots_children.push_back(std::move(root));
          root = std::move(tree_type(tree_val, std::move(new_roots_children)));
        }
        rightmost_path.push_back(&root);
      } else {
        tree_type &new_parent = *rightmost_path.back();
        if (!new_parent.children().empty() &&
            new_parent.id().second < new_parent.children().back().id().second) {
          {
            std::vector<tree_type> new_subtrees_children;
            new_subtrees_children.push_back(std::move(new_parent.children().back()));
            new_parent.children().back() = std::move(tree_type(tree_val, std::move(new_subtrees_children)));
          }
        } else {
          new_parent.children().push_back(tree_type(tree_val));
        }
        rightmost_path.push_back(const_cast<tree_type *>(&new_parent.children().back()));
      }
    }
    return root;
  }

  void fill_idx_to_node(const tree_type &t) {
    _idx_to_node[t.id().second] = &t;
    std::for_each(t.children().begin(), t.children().end(),
                  [this](const tree_type &c) {
                    fill_idx_to_node(c);
                  });
  }

public:
  opt_rmq(iterator_type b, iterator_type e)
    : rmq<iterator_type, value_type, difference_type>(b, e),
      _idx_to_node(n()),
      _tree(cartesian_tree(b, e)),
      _lca(_tree)
  {
    fill_idx_to_node(_tree);
  }

  difference_type query(iterator_type u, iterator_type v) const {
    return _lca.query(*_idx_to_node[u - begin()], *_idx_to_node[v - 1 - begin()]).second;
  }
};
