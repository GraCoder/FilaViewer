#pragma once

#include "Shape.h"

namespace fv {

class Cube : public Shape {
public:
  Cube(filament::math::float3 center, filament::math::float3 hf);
  ~Cube();

  virtual filament::Box box() const override;

  virtual Mesh mesh() override;

private:
  filament::math::float3 _center;
  filament::math::float3 _half;
};

} // namespace fv
