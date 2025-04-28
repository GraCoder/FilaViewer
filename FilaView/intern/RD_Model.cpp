#include "RD_Model.h"

#include "MeshAssimp.h"

RD_Model::RD_Model(ModelNode *node)
  : RDNode(node)
{
}

RD_Model::~RD_Model() {}

void RD_Model::build(filament::Engine *engine, const filament::Material *basicmtl, const filament::Material *defmtl) 
{
  if (!_assimp)
    _assimp = std::make_unique<MeshAssimp>();

  auto node = static_cast<ModelNode*>(_node);
  if (!_assimp->load_assert(node->file().c_str()))
    return;

  _assimp->build_assert(engine, basicmtl, defmtl);

  _entities.clear();
  for (auto ent : _assimp->renderables()) {
    _entities.push_back(ent.getId());
  }

  //if (sz) {
  //  auto &tcm = _engine->getTransformManager();
  //  auto ti = tcm.getInstance(_assimp->root());
  //  auto &mi = _assimp->min_bound();
  //  auto &ma = _assimp->max_bound();
  //  auto m = ma - mi;
  //  auto f = std::max({m.x, m.y, m.z});
  //  tcm.setTransform(ti, mat4::scaling(sz / f) * mat4::translation((mi + ma) / 2.0));
  //}
}
