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

  void exec(bool thread);

  filament::Engine *engine() { return _engine; }
  filament::SwapChain *getSwapChain() { return _swapchain; }
  filament::Renderer *getRenderer() { return _renderer; }

  SDL_Window *getSDLWindow() { return _window; }

  void setup_gui();

  void set_flags(uint32_t flags);

private:

  void create_window();

  void realize_render();

  void create_engine();

  void configure_cameras();

  void fixup_mouse_coord(int& x, int& y) const;

  void poll_events();

  inline FTView *view() { return _view.get(); }

private:

  bool _close = false;
  bool _render_realized = false;

  uint32_t _win_flags = 0;

  SDL_Window *_window = nullptr;

  filament::Engine *_engine = nullptr;
  filament::SwapChain *_swapchain = nullptr;
  filament::Renderer *_renderer = nullptr;

  filament::View *_gui_view = nullptr;
  filagui::ImGuiHelper *_gui = nullptr;

  std::shared_ptr<FTView> _view = nullptr;

  std::thread _thread;
};