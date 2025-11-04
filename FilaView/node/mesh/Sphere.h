#pragma once
#include "Shape.h"

namespace fv {

class Sphere : public Shape {
public:
  Sphere(const filament::math::float3 &center, float radius, int subDiv = 3);
  ~Sphere();

  Sphere(Sphere const &) = delete;
  Sphere &operator=(Sphere const &) = delete;

  virtual filament::Box box() const override;

  virtual Mesh mesh() override;

private:
  filament::math::float3 _center;
  float   _radius;
  uint8_t _subDiv = 2;
};

} // namespace fv
