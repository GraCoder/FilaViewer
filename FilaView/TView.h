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

  std::shared_ptr<TScene> scene() { return _scene; };

  using Manipulator = filament::camutils::Manipulator<float>;
  Manipulator *manip();

  void set_manip_factor(float f);

  void process(float delta);

  void zoom_box(const tg::boundingbox &box);

  std::optional<tg::vec3d> get_pos(int x, int y);

protected:

  TView();

protected:
  bool _cam_dirty = true;
  float _near = 0.4, _far = 2000;

  std::shared_ptr<TScene> _scene = nullptr;
};