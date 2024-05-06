#pragma once

#include <optional>
#include <memory>
#include <math/vec2.h>

#include <camutils/Manipulator.h>

#include "tmath.h"

class TWin;
class TScene;

class TView {
public:

  static std::shared_ptr<TView> create(TWin *win);

  ~TView();

  using Manipulator = filament::camutils::Manipulator<float>;
  Manipulator *manip();

  void set_manip_factor(float f);

  void zoom_box(const tg::boundingbox &box);

  std::optional<tg::vec3d> get_pos(int x, int y);

protected:

  TView();

protected:
  bool _cam_dirty = true;
  float _near = 0.4, _far = 2000;
};