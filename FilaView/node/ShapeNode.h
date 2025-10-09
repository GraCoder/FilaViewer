#pragma once

#include <vector>
#include <math/mat4.h>
#include <utils/Entity.h>

#include "node/Node.h"
#include "tvec.h"

namespace filament{
class Camera;
class VertexBuffer;
class IndexBuffer;
class Material;
class MaterialInstance;
}

namespace fv {

class Shape;

class ShapeNode : public Node {
public:
  ShapeNode(std::unique_ptr<Shape> &&shape)
    : _shape(std::move(shape))
  {
  }

  void build(filament::Engine *engine, filament::Material const *material);
  void release(filament::Engine *engine) override;

  utils::Entity solidEntity() { return _solid_entity; }
  utils::Entity wire_entity() { return _wire_entity; }

  void mapFrustum(filament::Engine &engine, filament::Camera const *camera);
  void mapFrustum(filament::Engine &engine, filament::math::mat4 const &transform);
  void mapAabb(filament::Engine &engine, filament::Box const &box);

private:
  std::unique_ptr<Shape> _shape;

  filament::Engine *_engine = nullptr;
  filament::VertexBuffer *_vert_buf = nullptr;
  filament::IndexBuffer *_index_buf = nullptr;
  filament::MaterialInstance *_solid_instance = nullptr;
  filament::MaterialInstance *_wire_instance = nullptr;
  utils::Entity _solid_entity;
  utils::Entity _wire_entity;

  std::vector<filament::math::float3>   _vertexs;
  std::vector<filament::math::short4>   _tangents;
  std::vector<uint16_t>                 _indices;
};

} // namespace fv
