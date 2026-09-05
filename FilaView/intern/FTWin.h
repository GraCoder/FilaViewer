#pragma once

#include "TWin.h"

#include <atomic>
#include <thread>

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

namespace fv {

class FTView;
class TOperator;
class ManipOperator;

class FTWin : public TWin
{
  friend class TWin;
public:
  FTWin(FTWin *);
  ~FTWin();

  uint64_t exec(bool thread);

  filament::Engine *engine() { return _engine; }
  filament::SwapChain *swapchain() { return _swapchain; }
  filament::Renderer *render() { return _renderer; }

  SDL_Window *window() { return _window; }

  inline FTView *view(int id = 0) { return _view.get(); }

  const std::shared_ptr<ManipOperator> &manip() { return _manip; }
  std::vector<std::shared_ptr<TOperator>> &operators() { return _operators; }

private:
  void createWindow(bool thread);

  void createEngine();

  void realizeContext();

  void configCamera();

  void clean();

  void setupGui();

  void pollEvents();

private:
  void gui(filament::Engine *, filament::View *);

private:
  std::atomic_bool _close = false;
  bool _realized = false;

  SDL_Window *_window = nullptr;
  uint64_t _nativeWindow = 0;

  filament::Engine *_engine = nullptr;
  filament::SwapChain *_swapchain = nullptr;
  filament::Renderer *_renderer = nullptr;

  filament::View *_guiView = nullptr;

  float _fps = 0;
  filagui::ImGuiHelper *_gui = nullptr;

  std::shared_ptr<FTView> _view = nullptr;

  std::shared_ptr<ManipOperator> _manip = nullptr;
  std::vector<std::shared_ptr<TOperator>> _operators;

  std::unique_ptr<std::thread> _thread;
};

} // namespace fv
