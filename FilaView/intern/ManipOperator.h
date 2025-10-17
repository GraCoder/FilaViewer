#pragma once

#include <math/vec3.h>
#include <camutils/Manipulator.h>

#include "tvec.h"
#include "TOperator.h"

namespace fv {

class ManipOperator : public TOperator {
  friend class FTView;

public:
  ManipOperator(FTView *);
  ~ManipOperator();

  void setPivot(const tg::vec3d &pos, double dis = 100);
  void setViewport(int w, int h);

public:
  bool mousePress(const SDL_MouseButtonEvent &btn) override;
  bool mouseRelease(const SDL_MouseButtonEvent &btn) override;
  bool mouseWheel(const SDL_MouseWheelEvent &wheel) override;
  bool mouseMove(const SDL_MouseMotionEvent &mov) override;
  bool keyPress(const SDL_KeyboardEvent &key) override;
  bool keyRelease(const SDL_KeyboardEvent &key) override;

  void getLookAt(filament::math::float3 &, filament::math::float3 &, filament::math::float3 &);

private:
  FTView *_view = nullptr;

  int _width = 0, _height = 0;
  bool _grabing = false;
  using Manipulator = filament::camutils::Manipulator<float>;
  Manipulator *_manip = nullptr;
};

} // namespace fv