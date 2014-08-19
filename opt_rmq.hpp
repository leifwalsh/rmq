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

  /**
   * Our tree will store pairs of the input element, and the offset within
   * the input represented by a node, so that once we compute the LCA in
   * the tree, we can convert that node back to an offset within the
   * original RMQ array.
   */
  typedef std::pair<value_type, difference_type> tree_val_type;
  typedef tree<tree_val_type> tree_type;

  /**
   * Array associating indexes to tree nodes, so we can find the tree
   * nodes corresponding to query offsets.
   */
  std::vector<const tree_type *> _idx_to_node;

  /**
   * Cartesian tree of the input array.
   */
  tree_type _tree;

  /**
   * Preprocessed LCA data structure answering queries on _tree.
   */
  lca<tree_val_type, pm_rmq<std::vector<ssize_t>::const_iterator> > _lca;

  /**
   * Constructs the Cartesian tree for the input array.
   */
  static tree_type cartesian_tree(iterator_type b, iterator_type e) {
    tree_type root(tree_val_type(*b, 0));

    // Our tree data structure doesn't maintain parent pointers, so we
    // maintain parents temporarily with an explicit stack.
    std::vector<tree_type *> rightmost_path;
    rightmost_path.push_back(&root);

    for (iterator_type c = b + 1; c != e; ++c) {
      // Backtrack up the rightmost path until our current value is larger
      // than the current value at the bottom of rightmost_path.
      while (!rightmost_path.empty() && rightmost_path.back()->id().first > *c) {
        rightmost_path.pop_back();
      }

      // This is the value (user value and offset) to be stored in the
      // tree node we're about to construct.
      tree_val_type tree_val(*c, c - b);

      if (rightmost_path.empty()) {
        // Our element is smaller than the current root's element.  We
        // need to construct a new root and make the old root its left
        // child.
        {
          std::vector<tree_type> new_roots_children;
          new_roots_children.push_back(std::move(root));
          root = std::move(tree_type(std::move(tree_val), std::move(new_roots_children)));
        }
        rightmost_path.push_back(&root);
      } else {
        // We need to create a new right child of the bottom of our
        // rightmost_path.  If it already has a right child, we need to
        // make that our new node's left child to preserve the inorder
        // traversal property.  If it doesn't have a right child, we can
        // just insert a leaf node.
        tree_type &new_parent = *rightmost_path.back();
        if (!new_parent.children().empty() &&
            // Since our tree is n-ary, it doesn't have a concept of
            // "left" and "right" children.  We use the inorder traversal
            // property we're supposed to be maintaining to check whether
            // a right child exists.  If it does, new_parent's RMQ input
            // index will be smaller than its last child's.
            new_parent.id().second < new_parent.children().back().id().second) {
          // We have a right child.  We need to replace it with our new node which must have the old right child as its left child.
          {
            std::vector<tree_type> new_subtrees_children;
            new_subtrees_children.push_back(std::move(new_parent.children().back()));
            new_parent.children().back() = std::move(tree_type(std::move(tree_val), std::move(new_subtrees_children)));
          }
        } else {
          // No right child, we can insert a fresh leaf node there.
          new_parent.children().push_back(std::move(tree_type(std::move(tree_val))));
        }
        // Either way, we just added a new node on the rightmost path.
        rightmost_path.push_back(const_cast<tree_type *>(&new_parent.children().back()));
      }
    }
    return root;
  }

  /**
   * We would be able to construct this array while building the Cartesian
   * tree if our tree nodes were immutable, but we're using std::move so
   * we have to wait until the tree is finished reshaping before computing
   * this array.
   */
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
    // To query, we use the query iterators' offsets and _idx_to_node to
    // find their corresponding nodes in the tree, run an LCA query on
    // those nodes, and report the returned node's offset.
    return _lca.query(*_idx_to_node[u - begin()],
                      *_idx_to_node[v - 1 - begin()]).second;
  }
};
