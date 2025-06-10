#pragma once

#include <memory>
#include <absl/container/inlined_vector.h>

class Node;

namespace filament {
class Engine;
}

class RDNode {
  constexpr static int entity_size = 4;
public:
  RDNode(Node *node) : _node(node) {}

  using EntityVector = absl::InlinedVector<uint32_t, entity_size>;
  const EntityVector &renderables() { return _entities; };

  virtual void update(double timestamp) {};

  virtual void release(filament::Engine *engine);

protected:

  Node *_node = nullptr;

  EntityVector _entities;
};