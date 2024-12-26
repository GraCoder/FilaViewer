#pragma once

#include <memory>
#include <string>

class RDNode;

class Node : public std::enable_shared_from_this<Node> {
public:
  Node();
  ~Node();

  uint32_t id() { return _id; }

  const std::unique_ptr<RDNode> &get_rd() { return _rd; }
  virtual const std::unique_ptr<RDNode> &get_rd(bool create) { return nullptr; }

protected:
  uint32_t _id = 0;
  std::unique_ptr<RDNode> _rd;
};