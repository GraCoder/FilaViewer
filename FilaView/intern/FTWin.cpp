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

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "ManipOperator.h"

#include "imgui/ImGuiHelper.h"
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
  fv::FTWin *win = new fv::FTWin(static_cast<fv::FTWin *>(s));
  if(!with_border)
    win->setFlags((uint32_t)fv::TWin::en_Frameless);
  return win;
}

void IWin::destroy(IWin *win) 
{
  fv::FTWin *w = static_cast<fv::FTWin *>(win);
  delete w;
}

namespace fv {

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
  _close = true;
  if (_thread)
    _thread->join();

  clean();
}

uint64_t FTWin::handle()
{
  if (!_window)
    createWindow();

  auto hwnd = native_window(_window);
  return (uint64_t)hwnd;
}

void FTWin::exec(bool thread)
{
  if (!_window)
    createWindow();

  if (thread) {
    _thread = std::make_unique<std::thread>(std::bind(&FTWin::pollEvents, this));
  } else
    pollEvents();
}

void FTWin::configCamera()
{
  float dpiScaleX = 1.0f;
  float dpiScaleY = 1.0f;

  // SDL_GL_GetDrawableSize(_window, &_width, &_height);
  uint32_t width = _width, height = _height;

  int virtualWidth, virtualHeight;
  SDL_GetWindowSize(_window, &virtualWidth, &virtualHeight);
  dpiScaleX = (float)width / virtualWidth;
  dpiScaleY = (float)height / virtualHeight;

  _view->manip()->setPivot({0, 0, 0}, 15);
  _view->setViewport(0, 0, width, height);
}

void FTWin::clean()
{
  if (_gui) {
    delete _gui;
    _gui = nullptr;
  }

  if (_gview) {
    _engine->destroy(_gview);
    _gview = nullptr;
  }

  if (_renderer) {
    _engine->destroy(_renderer);
    _renderer = nullptr;
  }

  if (_swapchain) {
    _engine->destroy(_swapchain);
    _swapchain = nullptr;
  }

  if (_view)
    _view.reset();

  if (_engine) {
    filament::Engine::destroy(_engine);
    _engine = nullptr;
  }
}

void FTWin::setupGui()
{
  if (_gui)
    return;

  using namespace filagui;

  _gview = _engine->createView();
  _gview->setViewport({0, 0, _width, _height});

  _gui = new ImGuiHelper(_engine, _gview, "");
  _gui->setDisplaySize(_width, _height);

  ImGuiIO &io = ImGui::GetIO();
#ifdef WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(_window, &wmInfo);
  // io.ImeWindowHandle = wmInfo.info.win.window;
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

void FTWin::createWindow()
{
  const int x = SDL_WINDOWPOS_CENTERED;
  const int y = SDL_WINDOWPOS_CENTERED;

  uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
  if (_flags & en_Frameless)
    flags |= SDL_WINDOW_BORDERLESS;
  auto win = SDL_CreateWindow("", x, y, _width, _height, flags);
  SDL_SetWindowResizable(win, SDL_TRUE);
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

  _window = win;
}

void FTWin::createEngine()
{
  using namespace filament;

  backend::Backend backend = Engine::Backend::VULKAN;
  // Engine::Config engineConfig = {};

  _engine = Engine::Builder()
    .backend(backend) 
    //.config(&engineConfig)
    .build();
  if (_engine)
    return;

  backend = Engine::Backend::OPENGL;
  _engine = Engine::Builder()
    .backend(backend) 
    //.config(&engineConfig)
    .build();
}

void FTWin::realizeContext()
{
  if (_realized)
    return;

  _realized = true;

  createEngine();

  _view->realize(_engine);

  configCamera();

  _swapchain = _engine->createSwapChain(native_window(_window), filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);
  _renderer = _engine->createRenderer();

  //{
  //  auto opts = _renderer->getClearOptions();
  //  opts.clear = true;
  //  opts.clearColor = filament::float4(1, 0, 0, 1);
  //  _renderer->setClearOptions(opts);
  //}

  _operators.emplace_back(_view->manip());

  if (_flags & en_SetupGui)
    setupGui();
}

#define OperIter for (auto iter = _operators.rbegin(); iter != _operators.rend(); iter++) (*iter)
void FTWin::pollEvents()
{
  uint32_t freq = SDL_GetPerformanceFrequency() / 1000;
  uint64_t stamp = SDL_GetPerformanceCounter(), prev_time = stamp;

  static constexpr int mk[4] = {0, 0, 1, 2};
  constexpr int max_event = 16;
  SDL_Event events[max_event];

  while (!_close) {
    if (!UTILS_HAS_THREADING)
      _engine->execute();

    int eventCount = 0;
    bool immediate = false;

    while (eventCount < max_event && SDL_PollEvent(&events[eventCount++]) != 0) {
    }
    for (int i = 0; i < eventCount; i++) {
      const SDL_Event &event = events[i];
      switch (event.type) {
      case SDL_QUIT: {
        view()->release();
        _close = true;
        break;
      }
      case SDL_KEYDOWN:
        OperIter->keyPress(event.key);
        break;
      case SDL_KEYUP:
        OperIter->keyRelease(event.key);
        break;
      case SDL_MOUSEWHEEL: {
        OperIter->mouseWheel(event.wheel);
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
        OperIter->mousePress(event.button);
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
        OperIter->mouseRelease(event.button);
        break;
      }
      case SDL_MOUSEMOTION: {
        if (_gui) {
          auto &io = ImGui::GetIO();
          io.AddMousePosEvent(event.motion.x, event.motion.y);
          if (io.WantCaptureMouse)
            break;
        }
        OperIter->mouseMove(event.motion);
        break;
      }
      case SDL_DROPFILE:
        // SDL_free(event.drop.file);
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
          // case SDL_WINDOWEVENT_RESIZED:
          {
            immediate = true;
            uint32_t w = event.window.data1;
            uint32_t h = event.window.data2;
            resize(w, h);
            if (_gui) {
              _gui->setDisplaySize(w, h);
            }
            if (_gview) {
              _gview->setViewport({0, 0, w, h});
            }
            break;
          }
        case SDL_WINDOWEVENT_EXPOSED:
        case SDL_WINDOWEVENT_SHOWN: {
          if (!_realized)
            realizeContext();
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

    if (!_realized)
      continue;

    // SDL_DisplayMode mode;
    // if(SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(_window), &mode) == 0 && mode.refresh_rate != 0) {
    //   round(1000.0 / mode.refresh_rate);
    // }

    uint64_t now = SDL_GetPerformanceCounter();
    double timestamp = (now - stamp) / freq;
    view()->process(timestamp);

    if (_gui) {
      ImGui::GetIO().DeltaTime = timestamp;
      _gui->render(timestamp, std::bind(&FTWin::gui, this, std::placeholders::_1, std::placeholders::_2));
    }

    if (!immediate) {
      uint32_t interval = uint32_t(now - prev_time) / freq;
      if (interval < 10) {
        SDL_Delay(10 - interval);
      }
    }

    prev_time = SDL_GetPerformanceCounter();
    if (_renderer->beginFrame(swapchain())) {
      _renderer->render(*_view);

      if (_gview) {
        _renderer->render(_gview);
      }

      _renderer->endFrame();
      //_renderer->readPixels();
    }
  }

  clean();
}

void FTWin::gui(filament::Engine *, filament::View *)
{
  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
  ImGui::Begin("Panel");
  if (ImGui::Button("Add Cube"))
    _view->scene()->addShape(0);
  if (ImGui::Button("Add Sphere"))
    _view->scene()->addShape(1);
  ImGui::End();
}

} // namespace fv
