#pragma once

#include "TWin.h"

#include <thread>

class FTView;
class TOperator;
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
  filament::SwapChain *swapchain() { return _swapchain; }
  filament::Renderer *render() { return _renderer; }

  SDL_Window *window() { return _window; }

  inline FTView *view(int id = 0) { return _view.get(); }

  std::vector<std::shared_ptr<TOperator>> &operators() { return _operators; }

private:

  void create_window();

  void realize_context();

  void create_engine();

  void configure_cameras();

  void setup_gui();

  void poll_events();

private:

  void gui(filament::Engine *, filament::View *);

private:

  bool _close = false;
  bool _realized = false;

  SDL_Window *_window = nullptr;

  filament::Engine *_engine = nullptr;
  filament::SwapChain *_swapchain = nullptr;
  filament::Renderer *_renderer = nullptr;

  filament::View *_gui_view = nullptr;
  filagui::ImGuiHelper *_gui = nullptr;

  std::shared_ptr<FTView> _view = nullptr;

  std::vector<std::shared_ptr<TOperator>> _operators;

  std::thread _thread;
};