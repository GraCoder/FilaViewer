#pragma once

#include <SDL2/SDL_events.h>

namespace fv {

class TOperator {
public:
  TOperator();

  ~TOperator();

public:
  virtual bool mousePress(const SDL_MouseButtonEvent &btn) { return false; }
  virtual bool mouseRelease(const SDL_MouseButtonEvent &btn) { return false; }
  virtual bool mouseWheel(const SDL_MouseWheelEvent &wheel) { return false; }
  virtual bool mouseMove(const SDL_MouseMotionEvent &mov) { return false; }
  virtual bool keyPress(const SDL_KeyboardEvent &key) { return false; }
  virtual bool keyRelease(const SDL_KeyboardEvent &key) { return false; }
};

} // namespace fv} // namespace fv