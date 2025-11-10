#pragma once

#include <memory>
#include <SDL2/SDL_events.h>

namespace fv {

class TView;

class TOperator {
public:
  TOperator() {}
  ~TOperator() {}

  virtual bool handle(TView *view, const SDL_Event *event);

public:
  virtual bool mousePress(TView *view, const SDL_MouseButtonEvent &btn) { return false; }
  virtual bool mouseRelease(TView *view, const SDL_MouseButtonEvent &btn) { return false; }
  virtual bool mouseWheel(TView *view, const SDL_MouseWheelEvent &wheel) { return false; }
  virtual bool mouseMove(TView *view, const SDL_MouseMotionEvent &mov) { return false; }
  virtual bool keyPress(TView *view, const SDL_KeyboardEvent &key) { return false; }
  virtual bool keyRelease(TView *view, const SDL_KeyboardEvent &key) { return false; }
};

} // namespace fv