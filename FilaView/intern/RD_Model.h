#pragma once

#include <filament/Engine.h>
#include <vector>

#include "node/ModelNode.h"
#include "node/RDNode.h"

class MeshAssimp;

class RD_Model : public RDNode {
public:
  RD_Model(ModelNode *node);

  ~RD_Model();

  void build(filament::Engine *engine, const filament::Material *basicmtl, const filament::Material *defmtl);

  void release(filament::Engine *engine) override;

private:

  std::unique_ptr<MeshAssimp> _assimp;
};