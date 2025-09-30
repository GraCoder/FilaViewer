#pragma once

#include <string>
#include <vector>

#include <filament/Engine.h>
#include <filament/Material.h>

#include "node/node.h"

class MeshAssimp;

namespace fv {

class MeshNode : public Node {
public:
  MeshNode();

  ~MeshNode();

  void build(const std::string &file, filament::Engine *engine, const filament::Material *basicmtl, const filament::Material *defmtl);

  void release(filament::Engine *engine) override;

private:

  std::unique_ptr<MeshAssimp> _assimp;
};

} // namespace fv