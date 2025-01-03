#pragma once

#include <memory>
#include <boost/container/small_vector.hpp>

class Node;

class RDNode {
  constexpr static int size = 4;
public:
  RDNode(Node *node) : _node(node) {}

  const boost::container::small_vector<uint32_t, size> &get_renderables() { return _entities; };

  virtual void update(double timestamp) {};

  virtual void release() {};

protected:

  Node *_node = nullptr;

  boost::container::small_vector<uint32_t, size> _entities;
};