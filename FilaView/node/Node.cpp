#include <filament/Engine.h>
#include <utils/Entity.h>
#include "Node.h"

namespace fv {

static uint32_t id_ = 0;

Node::Node()
  : _id(++id_)
{
}

void Node::build(filament::Engine *engine, filament::Material const *material) {}

void Node::release(filament::Engine *engine)
{
  for (auto rd : _entities) {
    engine->destroy(utils::Entity::import(rd));
  }
  _entities.clear();
}

} // namespace fv