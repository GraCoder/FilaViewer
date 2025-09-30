#pragma once

#include <SDL2/SDL_events.h>

namespace fv {

class TOperator {
public:
  TOperator();

  ~TOperator();

public:

  virtual bool mouse_press(const SDL_MouseButtonEvent &btn) { return false; }
  virtual bool mouse_release(const SDL_MouseButtonEvent &btn) { return false; }
  virtual bool mouse_wheel(const SDL_MouseWheelEvent &wheel) { return false; }
  virtual bool mouse_move(const SDL_MouseMotionEvent &mov) { return false; }
  virtual bool key_press(const SDL_KeyboardEvent &key) { return false; }
  virtual bool key_release(const SDL_KeyboardEvent &key) { return false; }
};

} // namespace fv