/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <utils/EntityManager.h>
#include <math/norm.h>

#include <geometry/SurfaceOrientation.h>

#include <map>
#include <array>

#include "Sphere.h"

using namespace filament;
using namespace filament::math;
using namespace utils;

using TriangleList = std::vector<ushort3>;
using VertexList = std::vector<float3>;
using IndexedMesh = std::pair<VertexList, TriangleList>;

static constexpr float X = .525731112119133606f;
static constexpr float Z = .850650808352039932f;
static constexpr float N = 0.f;

const VertexList sVertices = {
  {-X, N, Z}, {X, N, Z}, 
  {-X, N, -Z}, {X, N, -Z}, 
  {N, Z, X}, {N, Z, -X}, 
  {N, -Z, X}, {N, -Z, -X}, 
  {Z, X, N}, {-Z, X, N}, 
  {Z, -X, N}, {-Z, -X, N}
};

const TriangleList sTriangles = {
  {1, 4, 0}, {4, 9, 0}, {4, 5, 9}, 
  {8, 5, 4}, {1, 8, 4}, {1, 10, 8}, 
  {10, 3, 8}, {8, 3, 5}, {3, 2, 5},  {3, 7, 2},  {3, 10, 7}, {10, 6, 7}, {6, 11, 7}, {6, 0, 11},
  {6, 1, 0}, {10, 1, 6}, {11, 0, 9}, {2, 11, 9}, {5, 2, 9},  {11, 2, 7}};


namespace {
using Lookup = std::map<std::pair<uint16_t, uint16_t>, uint16_t>;
uint16_t vertex_for_edge(Lookup &lookup, VertexList &vertices, uint16_t first, uint16_t second)
{
  Lookup::key_type key(first, second);
  if (key.first > key.second) {
    std::swap(key.first, key.second);
  }

  auto inserted = lookup.insert({key, (Lookup::mapped_type)vertices.size()});
  if (inserted.second) {
    auto edge0 = vertices[first];
    auto edge1 = vertices[second];
    auto point = normalize(edge0 + edge1);
    vertices.push_back(point);
  }

  return inserted.first->second;
}

TriangleList subdivide(VertexList &vertices, TriangleList const &triangles)
{
  Lookup lookup;
  TriangleList result;
  for (ushort3 const &each : triangles) {
    std::array<uint16_t, 3> mid;
    mid[0] = vertex_for_edge(lookup, vertices, each[0], each[1]);
    mid[1] = vertex_for_edge(lookup, vertices, each[1], each[2]);
    mid[2] = vertex_for_edge(lookup, vertices, each[2], each[0]);
    result.push_back({each[0], mid[0], mid[2]});
    result.push_back({each[1], mid[1], mid[0]});
    result.push_back({each[2], mid[2], mid[1]});
    result.push_back({mid[0], mid[1], mid[2]});
  }
  return result;
}

auto create_sphere(const float3 &pos, float sz, int subdiv = 5) 
{
  VertexList vertices = sVertices;
  TriangleList triangles = sTriangles;
  for (int i = 0; i < subdiv; ++i) {
    triangles = subdivide(vertices, triangles);
  }
  return std::make_tuple(vertices, triangles);
}

}

Sphere::Sphere(SphereNode *node)
  : RDShape(node)
{
}

Sphere::~Sphere()
{
  this->release();
}

void Sphere::build(filament::Engine *engine, filament::Material const *material)
{
  _engine = engine;
  auto sph = static_cast<SphereNode *>(_node);
  auto [vertices, indices] = create_sphere(*(float3 *)&sph->pos(), sph->radius());

  _tangents.resize(vertices.size());
  auto *quats = geometry::SurfaceOrientation::Builder().vertexCount(vertices.size()).normals(vertices.data(), sizeof(float3)).build();
  quats->getQuats((short4 *)_tangents.data(), vertices.size(), sizeof(filament::math::short4));
  delete quats;

  _vert_buf = VertexBuffer::Builder()
                    .vertexCount((uint32_t)vertices.size())
                    .bufferCount(2)
                    .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                    .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
                    .normalized(VertexAttribute::TANGENTS)
                    .build(*engine);

  _vertexs = std::move(vertices);
  _indices = std::move(indices);
  _vert_buf->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(_vertexs.data(), _vertexs.size() * sizeof(float3)));
  _vert_buf->setBufferAt(*engine, 1, VertexBuffer::BufferDescriptor(_tangents.data(), _tangents.size() * sizeof(filament::math::short4)));

  uint32_t indexCount = (uint32_t)(_indices.size() * 3);
  _index_buf = IndexBuffer::Builder().bufferType(IndexBuffer::IndexType::USHORT).indexCount(indexCount).build(*engine);
  _index_buf->setBuffer(*engine, IndexBuffer::BufferDescriptor(_indices.data(), indexCount * sizeof(uint16_t)));

  if (material) {
    _mtl_instance = material->createInstance();
    _mtl_instance->setParameter("baseColor", RgbaType::LINEAR, LinearColorA(0.8, 0, 0, 1.0));
  }

  utils::EntityManager &em = utils::EntityManager::get();
  _entity = em.create();
  RenderableManager::Builder(1)
    .boundingBox({{0}, {1}})
    .material(0, _mtl_instance)
    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, _vert_buf, _index_buf)
    .culling(true)
    .build(*engine, _entity);

  _entities.clear();
  _entities.push_back(_entity.getId());
}

void Sphere::release() 
{
  if (_engine == nullptr)
    return;

  _engine->destroy(_vert_buf);
  _engine->destroy(_index_buf);
  _engine->destroy(_mtl_instance);
  _engine->destroy(_entity);

  _engine = nullptr;

  utils::EntityManager &em = utils::EntityManager::get();
  em.destroy(_entity);
}

Sphere &Sphere::setPosition(filament::math::float3 const &position) noexcept
{
  auto &tcm = _engine->getTransformManager();
  auto ci = tcm.getInstance(_entity);
  mat4f model = tcm.getTransform(ci);
  model[3].xyz = position;
  tcm.setTransform(ci, model);
  return *this;
}

Sphere &Sphere::setRadius(float radius) noexcept
{
  auto &tcm = _engine->getTransformManager();
  auto ci = tcm.getInstance(_entity);
  mat4f model = tcm.getTransform(ci);
  model[0].x = radius;
  model[1].y = radius;
  model[2].z = radius;
  tcm.setTransform(ci, model);
  return *this;
}
