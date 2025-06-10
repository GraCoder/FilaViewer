#include "RDNode.h"
#include <filament/Engine.h>
#include <utils/Entity.h>

void RDNode::release(filament::Engine *engine)
{
  for (auto rd : _entities) {
    engine->destroy(utils::Entity::import(rd));
  }
  _entities.clear();
}