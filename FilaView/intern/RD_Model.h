#pragma once

#include <vector>
#include <filament/Engine.h>

#include "node/RDNode.h"
#include "node/ModelNode.h"

class MeshAssimp;

class RD_Model : public RDNode {
public:
  RD_Model(const std::shared_ptr<ModelNode> &node);

  ~RD_Model();

  void build(filament::Engine *engine, const filament::Material *basicmtl, const filament::Material *defmtl);

private:

  std::unique_ptr<MeshAssimp> _assimp;
};