#include <functional>
#include <stdexcept>

#include <filament/Renderer.h>
#include <filament/Engine.h>
#include <filament/Options.h>
#include <filament/SwapChain.h>
#include <filament/Viewport.h>
#include <viewer/ViewerGui.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <camutils/Bookmark.h>

#include <viewer/ViewerGui.h>
#include "imgui/ImGuiHelper.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "imgui/imgui.h"

#include "FTView.h"
#include "FTScene.h"
#include "FTWin.h"

HWND native_window(SDL_Window *win)
{
  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  SDL_GetWindowWMInfo(win, &wmi);
  return wmi.info.win.window;
};

IWin *IWin::create(IWin *s, bool with_border)
{
  auto win = new FTWin(static_cast<FTWin *>(s));
  uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
  if(!with_border)
    flags |= SDL_WINDOW_BORDERLESS;
  win->set_flags(flags);
  return win;
}

FTWin::FTWin(FTWin *win)
  : TWin()
{
  if (SDL_Init(SDL_INIT_EVENTS))
    throw(std::runtime_error("sdl init failed."));

  _view = std::static_pointer_cast<FTView>(FTView::create(this));

  auto scene = std::static_pointer_cast<FTScene>(FTScene::create());
  _view->set_scene(scene);
}

FTWin::~FTWin()
{
  if (_gui)
    delete _gui;

  _view.reset();

  if (_swapchain)
    _engine->destroy(_swapchain);
  if (_renderer)
    _engine->destroy(_renderer);

  filament::Engine::destroy(_engine);
}

uint64_t FTWin::handle() 
{
  if (!_window)
    create_window();

  auto hwnd = native_window(_window);
  return (uint64_t)hwnd;
}

void FTWin::exec(bool thread) 
{
  if (!_window)
    create_window();

  if(thread) {
    _thread = std::thread(std::bind(&FTWin::poll_events, this));
  }
  else
    poll_events();
}

void FTWin::configure_cameras()
{
  float dpiScaleX = 1.0f;
  float dpiScaleY = 1.0f;

  //SDL_GL_GetDrawableSize(_window, &_width, &_height);
  uint32_t width = _width, height = _height;

  int virtualWidth, virtualHeight;
  SDL_GetWindowSize(_window, &virtualWidth, &virtualHeight);
  dpiScaleX = (float)width / virtualWidth;
  dpiScaleY = (float)height / virtualHeight;

  view()->set_viewport(0, 0, width, height);
}

void FTWin::fixup_mouse_coord(int& x, int& y) const
{
}

void FTWin::setup_gui() 
{
  if (_gui)
    return;
  using namespace filagui;

  _gui_view = _engine->createView();
  _gui_view->setViewport({0, 0, _width, _height});

  _gui = new ImGuiHelper(_engine, _gui_view, "");
  _gui->setDisplaySize(_width, _height);

  ImGuiIO &io = ImGui::GetIO();
#ifdef WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(_window, &wmInfo);
  //io.ImeWindowHandle = wmInfo.info.win.window;
#endif
  io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
  io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
  io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
  io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
  io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
  io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
  io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
  io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
  io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
  io.SetClipboardTextFn = [](void *, const char *text) { SDL_SetClipboardText(text); };
  io.GetClipboardTextFn = [](void *) -> const char * { return SDL_GetClipboardText(); };
  io.ClipboardUserData = nullptr;
}

void FTWin::set_flags(uint32_t flags) 
{
  _win_flags = flags;
}

void FTWin::create_window() 
{
  const int x = SDL_WINDOWPOS_CENTERED;
  const int y = SDL_WINDOWPOS_CENTERED;

  auto win = SDL_CreateWindow("", x, y, _width, _height, _win_flags);
  SDL_SetWindowResizable(win, SDL_TRUE);
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

  _window = win;
}

void FTWin::realize_render() 
{
  if (_render_realized)
    return;

  create_engine(); 

  _view->realize(_engine);

  configure_cameras();

  _view->set_pivot({0, 0, 0}, 15);

  _swapchain = _engine->createSwapChain(native_window(_window), filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);
  _renderer = _engine->createRenderer();

  setup_gui();

  _render_realized = true;
}

void FTWin::create_engine()
{
  using namespace filament;

  auto backend = Engine::Backend::VULKAN;

  // Engine::Config engineConfig = {};

  _engine = Engine::Builder() /*.config(&engineConfig)*/.build();
}

void FTWin::poll_events() 
{
  float freq = SDL_GetPerformanceFrequency() / 1000.0;
  uint64_t time = SDL_GetPerformanceCounter();

  static constexpr int mk[4] = {0, 0, 1, 2};

  while (!_close) {
    if (!UTILS_HAS_THREADING)
      _engine->execute();

    constexpr int max_event = 16;
    SDL_Event events[max_event];
    int ev_count = 0;
    while (ev_count < max_event && SDL_PollEvent(&events[ev_count++]) != 0) {
    }
    for (int i = 0; i < ev_count; i++) {
      const SDL_Event &event = events[i];
      switch (event.type) {
      case SDL_QUIT: {
        view()->clean();
        _close = true;
        break;
      }
      case SDL_KEYDOWN:
        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
          _close = true;
        } else if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL) {
          view()->set_manip_factor(10.0);
        } else if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
          view()->set_manip_factor(5.0);
        }
        view()->key_down(event.key.keysym.scancode);
        break;
      case SDL_KEYUP:
        view()->key_up(event.key.keysym.scancode);
        view()->set_manip_factor(1.0);
        break;
      case SDL_MOUSEWHEEL: {
        view()->mouse_wheel(event.wheel.mouseX, event.wheel.mouseY, event.wheel.preciseY);
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        if (_gui) {
          auto &io = ImGui::GetIO();
          io.AddMousePosEvent(event.button.x, event.button.y);
          io.AddMouseButtonEvent(mk[event.button.button], true);
          if (io.WantCaptureMouse)
            break;
        }

        view()->mouse_down(event.button.button, event.button.x, _height - event.button.y);
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        if (_gui) {
          auto &io = ImGui::GetIO();
          io.AddMousePosEvent(event.button.x, event.button.y);
          io.AddMouseButtonEvent(mk[event.button.button], false);
          if (io.WantCaptureMouse)
            break;
        }

        view()->mouse_up(event.button.x, _height - event.button.y);
        break;
      }
      case SDL_MOUSEMOTION: {
        if (_gui) {
          auto &io = ImGui::GetIO();
          io.AddMousePosEvent(event.motion.x, event.motion.y);
          if (io.WantCaptureMouse)
            break;
        }

        view()->mouse_move(event.motion.x, _height - event.motion.y);
        break;
      }
      case SDL_DROPFILE:
        // SDL_free(event.drop.file);
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESIZED: {
          uint32_t w = event.window.data1;
          uint32_t h = event.window.data2;
          resize(w, h);
          if (_gui) {
            _gui->setDisplaySize(w, h);
          }
          if(_gui_view) {
            _gui_view->setViewport({0, 0, w, h}); 
          }
          break;
        }
        case SDL_WINDOWEVENT_SHOWN: {
          if (!_render_realized)
            realize_render();
          break;
        }
        default:
          break;
        }
        break;
      default:
        break;
      }
    }

    uint64_t now = SDL_GetPerformanceCounter();
    double timestamp = (now - time) / freq;

    //if (_gui && view()->gui_callback()) {
    //  ImGui::GetIO().DeltaTime = timestamp;
    //  _gui->render(timestamp, view()->gui_callback());
    //}

    view()->process(timestamp);

    SDL_DisplayMode mode;
    int refresh_interval_ms =
      (SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(_window), &mode) == 0 && mode.refresh_rate != 0)
        ? round(1000.0 / mode.refresh_rate)
        : 16;
    SDL_Delay(refresh_interval_ms);

    if (_renderer->beginFrame(getSwapChain())) {
      _renderer->render(*view());
      _renderer->endFrame();
      //_renderer->readPixels();
    }
  }
}
