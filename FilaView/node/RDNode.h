#pragma once

#include <memory>
#include <boost/container/small_vector.hpp>

class Node;

class RDNode {
  constexpr static int size = 4;
public:
  RDNode(const std::shared_ptr<Node> &node) : _node(node) {}

  const boost::container::small_vector<uint32_t, size> &get_renderables() { return _entities; };

  virtual void update(double timestamp) {};

protected:

  std::weak_ptr<Node> _node;

  boost::container::small_vector<uint32_t, size> _entities;
};