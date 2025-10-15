#pragma once
#include "Shape.h"

namespace fv {

class Sphere : public Shape {
public:
  Sphere(const filament::math::float3 &center, float radius);
  ~Sphere();

  Sphere(Sphere const &) = delete;
  Sphere &operator=(Sphere const &) = delete;

  virtual filament::Box box() const override;

  virtual std::vector<filament::math::float3> vertexs() override;
  virtual std::vector<uint16_t> indexs() override;

private:
  filament::math::float3 _center;
  float _radius;
};

} // namespace fv
