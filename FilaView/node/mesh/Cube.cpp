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

static const float3 mVertices[] = {
  {-1, -1, -1}, {1, -1, -1}, {-1, 1, -1}, {1, 1, -1},
  {-1, -1, 1}, {1, -1, 1}, {-1, 1, 1}, {1, 1, 1}
};

static const uint32_t mIndices[] = {
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

static constexpr size_t WIREFRAME_OFFSET = 3 * 2 * 6;

static const uint32_t mIndices2[] = {
  2, 3, 0, 1,
  4, 5, 6, 7,
  0, 1, 4, 5,
  3, 2, 7, 6,
  2, 0, 6, 4,
  1, 3, 5, 7
};

namespace fv {

Cube::Cube() {}

Cube::~Cube()
{
}

} // namespace fv
