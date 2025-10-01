#include "node/ShapeNode.h"

#include <filament/Camera.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <utils/EntityManager.h>

namespace fv {

using namespace filament;
using namespace filament::math;

ShapeNode::ShapeNode(Shape *shape) {}

void ShapeNode::build(filament::Engine *engine, filament::Material const *material)
{
  _engine = engine;
  _vert_buf = VertexBuffer::Builder()
                .vertexCount(24)
                .bufferCount(2)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
                .build(*engine);

  _index_buf = IndexBuffer::Builder().bufferType(IndexBuffer::IndexType::USHORT).indexCount(12 * 2 + 3 * 2 * 6).build(*engine);

  //////////if (material) {
  //////////  _solid_instance = material->createInstance();
  //////////  _wire_instance = material->createInstance();
  //////////  _solid_instance->setParameter("baseColor", RgbaType::LINEAR, LinearColorA(0.4, 0.41, 0.4, 1.0));
  //////////  _wire_instance->setParameter("baseColor", RgbaType::LINEAR, LinearColorA{0.0, 0.6, 0.0, 0.5});
  //////////}

  //////////auto cube = static_cast<ShapeNode*>(_node);
  //////////auto pos = *(float3 *)&cube->pos();
  //////////auto size = *(float3 *)&cube->size();
  //////////_vertexs.clear();
  //////////_vertexs.reserve(24);
  //////////for (int i = 0; i < sizeof(mIndices2) / sizeof(uint32_t); i++) {
  //////////  auto vert = mVertices[mIndices2[i]] * (size / 2.0) + pos;
  //////////  _vertexs.emplace_back(vert);
  //////////}

  //////////_indices.clear();
  //////////_indices.reserve(36 + 24);
  //////////for (int i = 0; i < sizeof(mIndices2) / sizeof(uint32_t); i += 4) {
  //////////  _indices.push_back(i);
  //////////  _indices.push_back(i + 3);
  //////////  _indices.push_back(i + 2);
  //////////  _indices.push_back(i);
  //////////  _indices.push_back(i + 1);
  //////////  _indices.push_back(i + 3);
  //////////}
  //////////{
  //////////  constexpr uint16_t lines[] = {0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7, 6, 7, 6, 4, 2, 4, 3, 5, 1, 7, 0, 6};
  //////////  _indices.insert(_indices.end(), lines, lines + 24);
  //////////}

  //////////auto *quats = geometry::SurfaceOrientation::Builder()
  //////////                .vertexCount(_vertexs.size())
  //////////                .positions(_vertexs.data(), sizeof(float3))
  //////////                .triangleCount(12)
  //////////                .triangles((ushort3 *)_indices.data())
  //////////                .build();
  //////////_tangents.resize(24);
  //////////quats->getQuats((short4 *)_tangents.data(), 24);
  //////////delete quats;

  //////////_vert_buf->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(_vertexs.data(), _vertexs.size() * sizeof(float3)));
  //////////_vert_buf->setBufferAt(*engine, 1, VertexBuffer::BufferDescriptor(_tangents.data(), _tangents.size() * sizeof(short4)));
  //////////_index_buf->setBuffer(*engine, IndexBuffer::BufferDescriptor(_indices.data(), _indices.size() * sizeof(uint16_t)));

  //////////utils::EntityManager &em = utils::EntityManager::get();
  //////////_solid_entity = em.create();
  //////////RenderableManager::Builder(1)
  //////////  .boundingBox({pos - size / 2.0, pos + size / 2.0})
  //////////  .material(0, _solid_instance)
  //////////  .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, _vert_buf, _index_buf, 0, 3 * 2 * 6)
  //////////  .priority(7)
  //////////  .culling(true)
  //////////  .build(*engine, _solid_entity);

  //////////_wire_entity = em.create();
  //////////RenderableManager::Builder(1)
  //////////  .boundingBox({pos - size / 2.0, pos + size / 2.0})
  //////////  .material(0, _wire_instance)
  //////////  .geometry(0, RenderableManager::PrimitiveType::LINES, _vert_buf, _index_buf, WIREFRAME_OFFSET, 24)
  //////////  .priority(6)
  //////////  .culling(false)
  //////////  .build(*engine, _wire_entity);

  //////////_entities.push_back(_solid_entity.getId());
  ////////////_entities.push_back(_wire_entity.getId());
}

void ShapeNode::release(filament::Engine *engine)
{
  if (_vert_buf) {
    engine->destroy(_vert_buf);
    _vert_buf = nullptr;
  }
  if (_index_buf) {
    engine->destroy(_index_buf);
    _index_buf = nullptr;
  }
  if (_solid_instance) {
    engine->destroy(_solid_instance);
    _solid_instance = nullptr;
  }
  if (_wire_instance) {
    engine->destroy(_wire_instance);
    _wire_instance = nullptr;
  }

  utils::EntityManager &em = utils::EntityManager::get();
  if (_solid_entity) {
    engine->destroy(_solid_entity);
    em.destroy(_solid_entity);
    _solid_entity = {};
  }

  if (_wire_entity) {
    engine->destroy(_wire_entity);
    em.destroy(_wire_entity);
    _wire_entity = {};
  }
}

void ShapeNode::mapFrustum(filament::Engine &engine, Camera const *camera)
{
  // the Camera far plane is at infinity, but we want it closer for display
  const mat4 vm(camera->getModelMatrix());
  mat4 p(vm * inverse(camera->getCullingProjectionMatrix()));
  return mapFrustum(engine, p);
}

void ShapeNode::mapFrustum(filament::Engine &engine, filament::math::mat4 const &transform)
{
  // the Camera far plane is at infinity, but we want it closer for display
  mat4f p(transform);
  auto &tcm = engine.getTransformManager();
  tcm.setTransform(tcm.getInstance(_solid_entity), p);
  tcm.setTransform(tcm.getInstance(_wire_entity), p);
}

void ShapeNode::mapAabb(filament::Engine &engine, filament::Box const &box)
{
  mat4 p = mat4::translation(box.center) * mat4::scaling(box.halfExtent);
  return mapFrustum(engine, p);
}

} // namespace fv
