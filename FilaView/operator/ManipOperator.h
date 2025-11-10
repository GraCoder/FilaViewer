#pragma once

#include <optional>
#include "TOperator.h"
#include "tvec.h"

namespace fv {

class FTView;
class ManipOperator : public TOperator {
public:
  ManipOperator();
  ~ManipOperator();

  void getLookAt(tg::vec3d &, tg::vec3d &, tg::vec3d &);

  void setViewer(FTView *view) { _view = view; }
  void setPivot(const tg::vec3d &pos, double dis = 10);
  void setViewport(int w, int h);

public:
  bool mousePress(TView *view, const SDL_MouseButtonEvent &btn) override;
  bool mouseRelease(TView *view, const SDL_MouseButtonEvent &btn) override;
  bool mouseMove(TView *view, const SDL_MouseMotionEvent &mov) override;
  bool mouseWheel(TView *view, const SDL_MouseWheelEvent &wheel) override;
  bool keyPress(TView *view, const SDL_KeyboardEvent &key) override;
  bool keyRelease(TView *view, const SDL_KeyboardEvent &key) override;

private:
  void performLeft(TView *view, float dx, float dy);
  void performRight(TView *view, float dx, float dy);

private:
  FTView *_view = nullptr;

  tg::vec3d _target{0, 0, 0};
  tg::quatd _rotation;
  double _distance = 100;

  struct MouseStamp {
    int x, y;
    uint8_t button;
  };
  MouseStamp _ms_t0, _ms_t1;

  std::optional<tg::vec3d> _upAxis, _rtAxis;
};

} // namespace fv