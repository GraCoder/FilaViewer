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

static float vertices[] = {
  // 前面 (4个顶点)
  -1.f, -1.f, 1.f, // 左下前 - 0
  1.f, -1.f, 1.f,  // 右下前 - 1
  1.f, 1.f, 1.f,   // 右上前 - 2
  -1.f, 1.f, 1.f,  // 左上前 - 3

  // 后面 (4个顶点)
  -1.f, -1.f, -1.f, // 左下后 - 4
  -1.f, 1.f, -1.f,  // 左上后 - 5
  1.f, 1.f, -1.f,   // 右上后 - 6
  1.f, -1.f, -1.f,  // 右下后 - 7

  // 上面 (4个顶点)
  -1.f, 1.f, -1.f, // 左后上 - 8
  -1.f, 1.f, 1.f,  // 左前上 - 9
  1.f, 1.f, 1.f,   // 右前上 - 10
  1.f, 1.f, -1.f,  // 右后上 - 11

  // 下面 (4个顶点)
  -1.f, -1.f, -1.f, // 左后下 - 12
  1.f, -1.f, -1.f,  // 右后下 - 13
  1.f, -1.f, 1.f,   // 右前下 - 14
  -1.f, -1.f, 1.f,  // 左前下 - 15

  // 右面 (4个顶点)
  1.f, -1.f, -1.f, // 右下后 - 16
  1.f, 1.f, -1.f,  // 右上后 - 17
  1.f, 1.f, 1.f,   // 右上前 - 18
  1.f, -1.f, 1.f,  // 右下前 - 19

  // 左面 (4个顶点)
  -1.f, -1.f, -1.f, // 左下后 - 20
  -1.f, -1.f, 1.f,  // 左下前 - 21
  -1.f, 1.f, 1.f,   // 左上前 - 22
  -1.f, 1.f, -1.f   // 左上后 - 23
};


static unsigned short indices[] = {
  // 前面 (2个三角形)
  0, 1, 2, // 第一个三角形
  2, 3, 0, // 第二个三角形

  // 后面 (2个三角形)
  4, 5, 6, // 第一个三角形
  6, 7, 4, // 第二个三角形

  // 上面 (2个三角形)
  8, 9, 10,  // 第一个三角形
  10, 11, 8, // 第二个三角形

  // 下面 (2个三角形)
  12, 13, 14, // 第一个三角形
  14, 15, 12, // 第二个三角形

  // 右面 (2个三角形)
  16, 17, 18, // 第一个三角形
  18, 19, 16, // 第二个三角形

  // 左面 (2个三角形)
  20, 21, 22, // 第一个三角形
  22, 23, 20  // 第二个三角形
};

namespace fv {

Cube::Cube(filament::math::float3 center, filament::math::float3 hf)
  : _center(center)
  , _half(hf)
{
}

Cube::~Cube() {}

filament::Box Cube::box() const
{
  filament::Box box;
  box.center = _center;
  box.halfExtent = _half;
  return box;
}

Cube::Mesh Cube::mesh()
{
  constexpr int vertexCount = sizeof(vertices) / (3 * sizeof(vertices[0]));
  std::vector<filament::math::float3> vertexs(vertexCount);
  filament::math::float3 *v = (filament::math::float3 *)vertices;
  for (int i = 0; i < vertexCount; i++) {
    vertexs[i] = v[i] * _half + _center;
  }

  std::vector<filament::math::ushort3> indexs;
  indexs.resize(sizeof(indices) / sizeof(indices[0]) / 3);
  memcpy(indexs.data(), indices, sizeof(indices));

 
  std::vector<filament::math::ushort4> tangents;
  auto *quats = geometry::SurfaceOrientation::Builder()
                  .vertexCount(vertexs.size())
                  .positions(vertexs.data(), sizeof(float3))
                  .triangleCount(indexs.size())
                  .triangles(indexs.data())
                  .build();
  tangents.resize(vertexs.size());
  quats->getQuats((short4 *)tangents.data(), vertexs.size());
  delete quats;

  return std::make_tuple(std::move(vertexs), std::move(tangents), std::move(indexs));
}

} // namespace fv
