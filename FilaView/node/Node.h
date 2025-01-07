#pragma once

#include <memory>

class RDNode;
class Node : public std::enable_shared_from_this<Node> {
public:
  Node();
  ~Node();

  uint32_t id() { return _id; }

  RDNode *get_rd() { return _rd; }
  virtual RDNode *get_rd(bool create) { return nullptr; }

protected:
  uint32_t _id = 0;
  RDNode *_rd = nullptr;
};