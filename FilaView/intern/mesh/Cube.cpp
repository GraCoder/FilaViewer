/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "Cube.h"

#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <geometry/SurfaceOrientation.h>
#include <utils/EntityManager.h>

using namespace filament::math;
using namespace filament;

const uint32_t Cube::mIndices[] = {
  // solid
  0, 2, 3,
  0, 3, 1, // far
  4, 7, 6,
  4, 5, 7, // near
  2, 0, 4,
  2, 4, 6, // left
  1, 3, 7,
  1, 7, 5, // right
  0, 1, 5,
  0, 5, 4, // bottom
  3, 6, 7,
  3, 2, 6, // top

  // wire-frame
  0, 1, 1, 3, 3, 2, 2, 0, // far 
  4, 5, 5, 7, 7, 6, 6, 4, // near
  0, 4, 1, 5, 3, 7, 2, 6,
};

const float3 Cube::mVertices[] = {
  {-1, -1, -1},
  {1, -1, -1},
  {-1, 1, -1},
  {1, 1, -1},
  {-1, -1, 1},
  {1, -1, 1}, 
  {-1, 1, 1}, 
  {1, 1, 1}
};

Cube::Cube(CubeNode *node)
  : RDShape(node)
{
}

Cube::~Cube()
{
  if (!mEngine)
    return;
  mEngine->destroy(mVertexBuffer);
  mEngine->destroy(mIndexBuffer);
  mEngine->destroy(mMaterialInstanceSolid);
  mEngine->destroy(mMaterialInstanceWireFrame);
  mEngine->destroy(mSolidRenderable);
  mEngine->destroy(mWireFrameRenderable);

  utils::EntityManager &em = utils::EntityManager::get();
  em.destroy(mSolidRenderable);
  em.destroy(mWireFrameRenderable);
}

void Cube::build(filament::Engine *engine, filament::Material const *material) 
{
  mEngine = engine;
  mMaterial = material;
  mVertexBuffer = VertexBuffer::Builder()
    .vertexCount(8)
    .bufferCount(2)
    .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
    .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
    .build(*engine);

  mIndexBuffer = IndexBuffer::Builder()
    .indexCount(12 * 2 + 3 * 2 * 6)
    .build(*engine);

  if (mMaterial) {
    mMaterialInstanceSolid = mMaterial->createInstance();
    mMaterialInstanceWireFrame = mMaterial->createInstance();
    mMaterialInstanceSolid->setParameter("baseColor", RgbaType::LINEAR, LinearColorA(0.8, 0.8, 0.8, 0.5));
    mMaterialInstanceWireFrame->setParameter("baseColor", RgbaType::LINEAR, LinearColorA{0.6, 0.6, 0.6, 0.25f});
  }

  std::vector<float3> tmpvec(3, float3(1, 0, 0));
  auto *quats = geometry::SurfaceOrientation::Builder()
    .vertexCount(3).normals(tmpvec.data(), sizeof(float3)).build();
  std::vector<short4> tmp;
  tmp.resize(4);
  quats->getQuats((short4 *)tmp.data(), 3, sizeof(short4));
  delete quats;

  auto cube = static_cast<CubeNode*>(_node);
  auto pos = *(math::float3 *)&cube->pos();
  auto size = *(math::float3 *)&cube->size();
  std::vector<filament::math::float3> vertices;
  vertices.reserve(8);
  for(int i = 0; i < 8; i++)
    vertices.push_back(mVertices[i] * (size / 2.0) + pos); 

  mVertexBuffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(vertices.data(), mVertexBuffer->getVertexCount() * sizeof(math::float3)));
  mIndexBuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(mIndices, mIndexBuffer->getIndexCount() * sizeof(uint32_t)));

  utils::EntityManager &em = utils::EntityManager::get();
  mSolidRenderable = em.create();
  RenderableManager::Builder(1)
    .boundingBox({vertices[0], vertices[7]})
    .material(0, mMaterialInstanceSolid)
    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffer, mIndexBuffer, 0, 3 * 2 * 6)
    .priority(7)
    .culling(true)
    .build(*engine, mSolidRenderable);

  mWireFrameRenderable = em.create();
  RenderableManager::Builder(1)
    .boundingBox({vertices[0], vertices[7]})
    .material(0, mMaterialInstanceWireFrame)
    .geometry(0, RenderableManager::PrimitiveType::LINES, mVertexBuffer, mIndexBuffer, WIREFRAME_OFFSET, 24)
    .priority(6)
    .culling(false)
    .build(*engine, mWireFrameRenderable);

  _entities.push_back(mSolidRenderable.getId());
  //_entities.push_back(mWireFrameRenderable.getId());
}

void Cube::mapFrustum(filament::Engine &engine, Camera const *camera)
{
  // the Camera far plane is at infinity, but we want it closer for display
  const mat4 vm(camera->getModelMatrix());
  mat4 p(vm * inverse(camera->getCullingProjectionMatrix()));
  return mapFrustum(engine, p);
}

void Cube::mapFrustum(filament::Engine &engine, filament::math::mat4 const &transform)
{
  // the Camera far plane is at infinity, but we want it closer for display
  mat4f p(transform);
  auto &tcm = engine.getTransformManager();
  tcm.setTransform(tcm.getInstance(mSolidRenderable), p);
  tcm.setTransform(tcm.getInstance(mWireFrameRenderable), p);
}

void Cube::mapAabb(filament::Engine &engine, filament::Box const &box)
{
  mat4 p = mat4::translation(box.center) * mat4::scaling(box.halfExtent);
  return mapFrustum(engine, p);
}
