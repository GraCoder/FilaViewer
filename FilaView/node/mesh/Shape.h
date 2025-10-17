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

  using Mesh = std::tuple<std::vector<filament::math::float3>, std::vector<filament::math::ushort4>, std::vector<filament::math::ushort3>>;
  virtual Mesh mesh() = 0;
};

} // namespace fv