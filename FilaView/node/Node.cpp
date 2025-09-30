#include "Node.h"
#include <filament/Engine.h>
#include <utils/Entity.h>

namespace fv {

static uint32_t id_ = 0;

Node::Node()
  : _id(++id_)
{
}

void Node::release(filament::Engine *engine)
{
  for (auto rd : _entities) {
    engine->destroy(utils::Entity::import(rd));
  }
  _entities.clear();
}

} // namespace fv