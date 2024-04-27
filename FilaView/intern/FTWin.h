#pragma once

#include "TWin.h"

#include <thread>

class FTView;
class SDL_Window;

namespace filament {
class View;
class Engine;
class Renderer;
class SwapChain;
} // namespace filament

namespace filagui {
class ImGuiHelper;
}

class FTWin : public TWin {
  friend class TWin;

public:

  FTWin(FTWin *);
  ~FTWin();

  uint64_t handle();

  void realize();

  void exec();

  filament::Engine *engine() { return _engine; }
  filament::SwapChain *getSwapChain() { return _swapchain; }
  filament::Renderer *getRenderer() { return _renderer; }

  SDL_Window *getSDLWindow() { return _window; }

  void setup_gui();

private:

  void create_engine();

  void configure_cameras();

  void fixup_mouse_coord(int& x, int& y) const;

  void poll_events();

  inline FTView *view() { return (FTView*)(_view.get()); }

private:

  SDL_Window *_window = nullptr;

  uint64_t _time = 0;

  filament::Engine *_engine = nullptr;
  filament::SwapChain *_swapchain = nullptr;
  filament::Renderer *_renderer = nullptr;

  //filament::View *_gui_view = nullptr;
  //filagui::ImGuiHelper *_gui = nullptr;

  std::thread _thread;
};