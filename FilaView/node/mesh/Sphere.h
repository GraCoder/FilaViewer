#pragma once
#include "Shape.h"

namespace fv {

class Sphere : public Shape {
public:
  Sphere();
  ~Sphere();

  Sphere(Sphere const &) = delete;
  Sphere &operator=(Sphere const &) = delete;
};

} // namespace fv
