#include <functional>
#include <stdexcept>

#include <filament/Camera.h>
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

#include "imgui/ImGuiHelper.h"
#include "imgui/imgui.h"

#include "FTView.h"
#include "FTScene.h"
#include "FTWin.h"
#include "operator/ManipOperator.h"

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

  _view = std::static_pointer_cast<FTView>(FTView::create());

  auto scene = std::static_pointer_cast<FTScene>(FTScene::create());
  _view->setScene(scene);

  _manip = std::make_shared<ManipOperator>();
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

  std::string fontPath;
  //fontPath = "c:/Windows/Fonts/simhei.ttf";
  _gui = new ImGuiHelper(_engine, _gview, fontPath);
  _gui->setDisplaySize(_width, _height);

  ImGuiIO &io = ImGui::GetIO();
#ifdef WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(_window, &wmInfo);
  // io.ImeWindowHandle = wmInfo.info.win.window;
#endif
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
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
  auto win = SDL_CreateWindow("", x, y, _width, _height, flags);
  SDL_SetWindowResizable(win, SDL_TRUE);
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

  _window = win;
}

void FTWin::createEngine()
{
  using namespace filament;

  _engine = Engine::Builder()
    .backend(Engine::Backend::VULKAN) 
    //.config(&engineConfig)
    .build();
  if (_engine)
    return;

  _engine = Engine::Builder()
    .backend(Engine::Backend::OPENGL) 
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

  if (_flags & en_SetupGui)
    setupGui();
}

#define OperIter for (auto iter = _operators.rbegin(); iter != _operators.rend(); iter++) handled |= (*iter)
void FTWin::pollEvents()
{
  uint64_t stampInit = SDL_GetPerformanceCounter(), stampPrev = stampInit;

  constexpr int max_event = 8;
  SDL_Event events[max_event];

  while (true) {
    int eventCount = 0;
    bool refresh = false, setCamera = false;
    double freq = SDL_GetPerformanceFrequency();

    while (eventCount < max_event && SDL_PollEvent(&events[eventCount++]) != 0) {}
    for (int i = 0; i < eventCount; i++) {
      bool handled = false;
      const SDL_Event &event = events[i];
      switch (event.type) {
      case SDL_QUIT: {
        view()->release();
        _close = true;
        break;
      }
      case SDL_KEYDOWN:
        if (_gui && _gui->keyDn(event.key))
          handled = true;
        else
          OperIter->keyPress(_view.get(), event.key);
        break;
      case SDL_KEYUP:
        if (_gui && _gui->keyUp(event.key))
          handled = true;
        else
          OperIter->keyRelease(_view.get(), event.key);
        break;
      case SDL_TEXTINPUT:
        if (_gui && _gui->inputText(event.text.text)) {}
        break;
      case SDL_MOUSEWHEEL: {
        if (_gui && _gui->mouseWheel(event.wheel))
          handled = true;
        else
          OperIter->mouseWheel(_view.get(), event.wheel);
        break;
      }
      case SDL_MOUSEBUTTONDOWN: {
        if (_gui && _gui->mouseButtonDn(event.button))
          handled = true;
        else
          OperIter->mousePress(_view.get(), event.button);
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        if (_gui && _gui->mouseButtonUp(event.button))
          handled = true;
        else
          OperIter->mouseRelease(_view.get(), event.button);
        break;
      }
      case SDL_MOUSEMOTION: {
        if (_gui && _gui->mouseMove(event.motion))
          handled = true;
        else
          OperIter->mouseMove(_view.get(), event.motion);
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
            refresh = true;
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
          setCamera = true;
          break;
        }
        default:
          break;
        }
        break;
      default:
        break;
      }

      if (!handled && _manip->handle(_view.get(), &event))
        setCamera = true;
    }

    if (_close)
      break;

    if (!_realized)
      continue;

    if (!UTILS_HAS_THREADING)
      _engine->execute();

    // SDL_DisplayMode mode;
    // if(SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(_window), &mode) == 0 && mode.refresh_rate != 0) {
    //   round(1000.0 / mode.refresh_rate);
    // }

    uint64_t counter = SDL_GetPerformanceCounter();

    {
      double timestamp = (counter - stampInit) / freq;
      auto refTime = timestamp * 1000.0;
      if ((_manip && _manip->process(refTime)) || setCamera) {
        filament::math::double3 eye, target, up;
        _manip->getLookAt(*(tg::vec3d *)&eye, *(tg::vec3d *)&target, *(tg::vec3d *)&up);
        static_cast<FTView *>(_view.get())->view()->getCamera().lookAt(eye, target, up);
      }
      view()->process(refTime);
    }

    if (_gui) {
      float delta = (counter - stampPrev) / freq;
      _gui->render(delta, std::bind(&FTWin::gui, this, std::placeholders::_1, std::placeholders::_2));
    }

    {
      static uint64_t fpsStamp = stampInit;
#ifndef NDEBUG
      if ((counter - fpsStamp) / freq > 1) {
        fpsStamp = counter;
        _fps = freq / double(counter - stampPrev);
      }
#endif
    }
    
    if (_renderer->beginFrame(swapchain())) {
      _renderer->render(*_view);

      if (_gview) {
        _renderer->render(_gview);
      }

      _renderer->endFrame();
      //_renderer->readPixels();
    }

    if (!refresh) {
      uint32_t interval = uint32_t(counter- stampPrev) / freq * 1000.0;
      if (interval < 14) {
        SDL_Delay(14 - interval);
      }
    }

    stampPrev = counter;
  }

  clean();
}

void FTWin::gui(filament::Engine *, filament::View *)
{
  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
  ImGui::Begin("Panel");
  if (_fps > 0)
    ImGui::LabelText("FPS", "%8.3f", _fps);
  if (ImGui::Button("Add Cube"))
    _view->scene()->addShape(0);
  if (ImGui::Button("Add Sphere"))
    _view->scene()->addShape(1);

  static char text[256] = {0};
  if(ImGui::InputText("123", text, 256)){
    printf("");
  }
  ImGui::End();
}

} // namespace fv
