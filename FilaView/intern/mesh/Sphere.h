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

#ifndef TNT_FILAMENT_SAMPLE_SPHERE_H
#define TNT_FILAMENT_SAMPLE_SPHERE_H

#include <math/vec3.h>
#include <utils/Entity.h>
#include <vector>

#include "RDShape.h"

namespace filament {
class Engine;
class IndexBuffer;
class Material;
class MaterialInstance;
class VertexBuffer;
} // namespace filament

class Sphere : public RDShape {
public:
  Sphere(SphereNode *node);
  ~Sphere();

  void build(filament::Engine *engine, filament::Material const *material) override;
  void release();

  Sphere(Sphere const &) = delete;
  Sphere &operator=(Sphere const &) = delete;

  utils::Entity solid_entity() const { return _entity; }

  filament::MaterialInstance *material_instance() { return _mtl_instance; }

  Sphere &setPosition(filament::math::float3 const &position) noexcept;
  Sphere &setRadius(float radius) noexcept;

private:
  filament::Engine *_engine = nullptr;
  filament::MaterialInstance *_mtl_instance = nullptr;

  filament::VertexBuffer *_vert_buf = nullptr;
  filament::IndexBuffer *_index_buf = nullptr;

  utils::Entity _entity;

  std::vector<filament::math::float3>   _vertexs;
  std::vector<filament::math::short4>   _tangents;
  std::vector<filament::math::ushort3>  _indices;
};

#endif // TNT_FILAMENT_SAMPLE_SPHERE_H
