#pragma once

#include <math/vec3.h>
#include <filament/box.h>
#include <vector>

namespace fv {

class Shape {
public:
  Shape();
  ~Shape();

  Shape(Shape const &) = delete;

  virtual filament::Box box() const = 0;

  virtual std::vector<filament::math::float3> vertexs() = 0;
  virtual std::vector<uint16_t> indexs() = 0;
};

} // namespace fv