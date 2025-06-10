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

#ifndef TNT_FILAMENT_SAMPLE_CUBE_H
#define TNT_FILAMENT_SAMPLE_CUBE_H

#include <vector>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/MaterialInstance.h>

#include "RDShape.h"

class Cube : public RDShape {
public:
  Cube(CubeNode *node);
  ~Cube();

  void build(filament::Engine *engine, filament::Material const *material) override;
  void release(filament::Engine *engine) override;

  utils::Entity solid_entity() { return _solid_entity; }
  utils::Entity wire_entity() { return _wire_entity; }

  void mapFrustum(filament::Engine &engine, filament::Camera const *camera);
  void mapFrustum(filament::Engine &engine, filament::math::mat4 const &transform);
  void mapAabb(filament::Engine &engine, filament::Box const &box);

private:
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

#endif // TNT_FILAMENT_SAMPLE_CUBE_H
