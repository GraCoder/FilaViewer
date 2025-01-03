#pragma once

#include "node/RDNode.h"
#include "node/ShapeNode.h"

#include <filament/Engine.h>
#include <filament/Material.h>
#include <utils/Entity.h>

class RDShape : public RDNode {
public:
  RDShape(ShapeNode *shape);
  ~RDShape();

  virtual void build(filament::Engine *engine, filament::Material const *material)  = 0;
}; 