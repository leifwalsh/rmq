#pragma once

#include <iostream>
#include <vector>

/**
 * Represents an n-ary tree with a node id and a vector of children.
 */
template<
  typename value_type
  >
class tree {

public:

  typedef typename std::vector<size_t>::difference_type index_type;

private:
  
  value_type _id;

  std::vector<tree> _children;

  index_type _repr;

public:

  tree() {}

  /**
   * Constructs a leaf node.
   */
  tree(const value_type &i)
    : _id(i),
      _children()
  {}

  tree(const value_type &i, const std::vector<tree> &c) = delete;

  /**
   * Constructs an internal node.
   */
  tree(const value_type &i, std::vector<tree> &&c)
    : _id(i),
      _children(std::move(c))
  {}

  tree(tree &&o) noexcept
    : _id(std::move(o._id)),
      _children(std::move(o._children)),
      _repr(std::move(o._repr))
  {}

  tree(const tree &o) = delete;

  tree& operator=(tree &&o) {
    _id = std::move(o._id);
    _children = std::move(o._children);
    _repr = std::move(o._repr);
    return *this;
  }

  tree& operator=(const tree &o) = delete;

  /**
   * This node's identifier.
   */
  const value_type &id() const { return _id; }

  /**
   * A list of this node's children.
   */
  std::vector<tree> &children() { return _children; }
  const std::vector<tree> &children() const { return _children; }

  /**
   * The node's representative index in the euler tour.  Used by the LCA
   * algorithm.
   */
  index_type repr() const { return _repr; }
  void set_repr(index_type r) { _repr = r; }
};

