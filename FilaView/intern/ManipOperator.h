#pragma once

#include <math/vec3.h>
#include <camutils/Manipulator.h>

#include "tvec.h"
#include "TOperator.h"

class ManipOperator : public TOperator {
  friend class FTView;
public:
  ManipOperator(FTView *);
  ~ManipOperator();

  void set_pivot(const tg::vec3d &pos, double dis = 100);
  void set_viewport(int w, int h);

public:
  bool mouse_press(const SDL_MouseButtonEvent &btn) override;
  bool mouse_release(const SDL_MouseButtonEvent &btn) override;
  bool mouse_wheel(const SDL_MouseWheelEvent &wheel) override;
  bool mouse_move(const SDL_MouseMotionEvent &mov) override;
  bool key_press(const SDL_KeyboardEvent &key) override;
  bool key_release(const SDL_KeyboardEvent &key) override;

  void get_lookat(filament::math::float3 &, filament::math::float3 &, filament::math::float3 &);

private:
  FTView *_view = nullptr;

  int _width = 0, _height = 0;
  bool _grabing = false;
  using Manipulator = filament::camutils::Manipulator<float>;
  Manipulator *_manip = nullptr;
};