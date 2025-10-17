#include "MeshNode.h"
#include "MeshAssimp.h"

namespace fv {

MeshNode::MeshNode()
  : Node()
{
}

MeshNode::~MeshNode() {}

void MeshNode::build(const std::string &file, filament::Engine *engine, const filament::Material *basicmtl, const filament::Material *defmtl)
{
  if (!_assimp)
    _assimp = std::make_unique<MeshAssimp>();

  if (!_assimp->loadAssert(file.c_str()))
    return;

  _assimp->buildAssert(engine, basicmtl, defmtl);

  _entities.clear();
  for (auto ent : _assimp->renderables()) {
    _entities.push_back(ent.getId());
  }

  // if (sz) {
  //   auto &tcm = _engine->getTransformManager();
  //   auto ti = tcm.getInstance(_assimp->root());
  //   auto &mi = _assimp->min_bound();
  //   auto &ma = _assimp->max_bound();
  //   auto m = ma - mi;
  //   auto f = std::max({m.x, m.y, m.z});
  //   tcm.setTransform(ti, mat4::scaling(sz / f) * mat4::translation((mi + ma) / 2.0));
  // }
}

void MeshNode::release(filament::Engine *engine)
{
  Node::release(engine);

  _assimp.reset();
}

} // namespace fv
