#include <functional>
#include <future>
#include <stdexcept>
#include <string>

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

#include <SDL3/SDL.h>

#include "imgui/ImGuiHelper.h"
#include "imgui/imgui.h"

#include "FTView.h"
#include "FTScene.h"
#include "FTWin.h"
#include "operator/ManipOperator.h"

void *native_window(SDL_Window *win)
{
  auto props = SDL_GetWindowProperties(win);
  if (props == 0)
    return nullptr;
  return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
}

IWin *IWin::create(IWin *s, bool)
{
  return new fv::FTWin(static_cast<fv::FTWin *>(s));
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
  _view = std::static_pointer_cast<FTView>(FTView::create());

  auto scene = std::static_pointer_cast<FTScene>(FTScene::create());
  _view->setScene(scene);

  scene->registerHandlers(_handlers);

  _manip = std::make_shared<ManipOperator>();
}

FTWin::~FTWin()
{
  _close = true;
  if (_thread)
    _thread->join();
}

uint64_t FTWin::exec(bool thread)
{
  std::promise<uint64_t> windowPromise;
  std::future<uint64_t> winId = windowPromise.get_future();

  auto run = [this, thread, promise = std::move(windowPromise)]() mutable {
    if (!SDL_Init(SDL_INIT_VIDEO))
      throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

    // An asynchronously hosted window starts hidden. The host shows it after
    // exec(true) returns, and the resulting SHOWN/EXPOSED event realizes Filament.
    createWindow(thread);

    void *nativeWindow = native_window(_window);
    if (!nativeWindow)
      throw std::runtime_error(std::string("Could not obtain the native window: ") + SDL_GetError());

    _nativeWindow = reinterpret_cast<uint64_t>(nativeWindow);
    promise.set_value(_nativeWindow);

    pollEvents();

    clean();

    if (_window) {
      SDL_DestroyWindow(_window);
      _window = nullptr;
    }

    _nativeWindow = 0;

    SDL_Quit();
  };

  if (thread) {
    _thread = std::make_unique<std::thread>(std::move(run));
    return winId.get();
  }

  run();
  return winId.get();
}

void FTWin::configCamera()
{
  if (_width == 0 || _height == 0)
    return;

  _view->setViewport(0, 0, _width, _height);
}

void FTWin::clean()
{
  if (_gui) {
    delete _gui;
    _gui = nullptr;
  }

  if (_guiView) {
    _engine->destroy(_guiView);
    _guiView = nullptr;
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

  _guiView = _engine->createView();
  _guiView->setViewport({0, 0, _width, _height});

  std::string fontPath;
  // fontPath = "c:/Windows/Fonts/simhei.ttf";
  _gui = new ImGuiHelper(_engine, _guiView, fontPath);
  _gui->setDisplaySize(_width, _height);

  ImGuiIO &io = ImGui::GetIO();
  // #ifdef WIN32
  //   SDL_SysWMinfo wmInfo;
  //   SDL_VERSION(&wmInfo.version);
  //   SDL_GetWindowWMInfo(_window, &wmInfo);
  //   // io.ImeWindowHandle = wmInfo.info.win.window;
  // #endif
  io.SetClipboardTextFn = [](void *, const char *text) { SDL_SetClipboardText(text); };
  io.GetClipboardTextFn = [](void *) -> const char * { return SDL_GetClipboardText(); };
  io.ClipboardUserData = nullptr;
}

void FTWin::createWindow(bool thread)
{
  uint32_t flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
  flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
  if (thread)
    flags |= SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS;

  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
  auto win = SDL_CreateWindow("FilaWin", _width, _height, flags);
  if (!win)
    throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

  _window = win;
  SDL_SyncWindow(win);
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

  createEngine();
  if (!_engine)
    throw std::runtime_error("Filament engine creation failed.");

  int pixelWidth = 0, pixelHeight = 0;
  if (!SDL_GetWindowSizeInPixels(_window, &pixelWidth, &pixelHeight))
    throw std::runtime_error(std::string("Could not query window pixel size: ") + SDL_GetError());
  _width = static_cast<uint32_t>(pixelWidth);
  _height = static_cast<uint32_t>(pixelHeight);

  _view->realize(_engine);

  configCamera();

  _swapchain = _engine->createSwapChain(reinterpret_cast<void *>(_nativeWindow), filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);
  _renderer = _engine->createRenderer();
  if (!_swapchain || !_renderer)
    throw std::runtime_error("Filament swap chain or renderer creation failed.");

  //{
  //  auto opts = _renderer->getClearOptions();
  //  opts.clear = true;
  //  opts.clearColor = filament::float4(1, 0, 0, 1);
  //  _renderer->setClearOptions(opts);
  //}

  setupGui();

  _realized = true;
}

#define OperIter                                                                                                                                               \
  for (auto iter = _operators.rbegin(); iter != _operators.rend(); iter++)                                                                                     \
  handled |= (*iter)
void FTWin::pollEvents()
{
  uint64_t stampInit = SDL_GetPerformanceCounter(), stampPrev = stampInit;

  constexpr int max_event = 8;
  SDL_Event events[max_event];

  while (true) {
    int eventCount = 0;
    bool refresh = false, setCamera = false;
    double freq = SDL_GetPerformanceFrequency();

    while (eventCount < max_event && SDL_PollEvent(&events[eventCount]))
      ++eventCount;
    for (int i = 0; i < eventCount; i++) {
      bool handled = false;
      const SDL_Event &event = events[i];
      switch (event.type) {
      case SDL_EVENT_QUIT: {
        view()->release();
        _close = true;
        break;
      }
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
        if (event.window.windowID == SDL_GetWindowID(_window)) {
          view()->release();
          _close = true;
        }
        break;
      }
      case SDL_EVENT_KEY_DOWN:
        if (_gui && _gui->keyDn(event.key))
          handled = true;
        else
          OperIter->keyPress(_view.get(), event.key);
        break;
      case SDL_EVENT_KEY_UP:
        if (_gui && _gui->keyUp(event.key))
          handled = true;
        else
          OperIter->keyRelease(_view.get(), event.key);
        break;
      case SDL_EVENT_TEXT_INPUT:
        if (_gui && _gui->inputText(event.text.text)) {
        }
        break;
      case SDL_EVENT_MOUSE_WHEEL: {
        if (_gui && _gui->mouseWheel(event.wheel))
          handled = true;
        else
          OperIter->mouseWheel(_view.get(), event.wheel);
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        if (_gui && _gui->mouseButtonDn(event.button))
          handled = true;
        else
          OperIter->mousePress(_view.get(), event.button);
        break;
      }
      case SDL_EVENT_MOUSE_BUTTON_UP: {
        if (_gui && _gui->mouseButtonUp(event.button))
          handled = true;
        else
          OperIter->mouseRelease(_view.get(), event.button);
        break;
      }
      case SDL_EVENT_MOUSE_MOTION: {
        if (_gui && _gui->mouseMove(event.motion))
          handled = true;
        else
          OperIter->mouseMove(_view.get(), event.motion);
        break;
      }
      case SDL_EVENT_DROP_FILE:
        // SDL_free(event.drop.file);
        break;
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        refresh = true;
        int pixelWidth = 0, pixelHeight = 0;
        if (SDL_GetWindowSizeInPixels(_window, &pixelWidth, &pixelHeight) && pixelWidth > 0 && pixelHeight > 0) {
          resize(pixelWidth, pixelHeight);
          if (_gui) {
            _gui->setDisplaySize(pixelWidth, pixelHeight);
          }
          if (_guiView) {
            _guiView->setViewport({0, 0, static_cast<uint32_t>(pixelWidth), static_cast<uint32_t>(pixelHeight)});
          }
        }
        break;
      }
      case SDL_EVENT_WINDOW_EXPOSED:
      case SDL_EVENT_WINDOW_SHOWN: {
        if (!_realized)
          realizeContext();
        refresh = true;
        setCamera = true;
        break;
      }
      default:
        break;
      }

      if (!handled && _manip->handle(_view.get(), &event))
        setCamera = true;
    }

    if (_close)
      break;

    if (!_realized) {
      SDL_Delay(1);
      continue;
    }

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

    if (_width > 0 && _height > 0 && _renderer->beginFrame(swapchain())) {
      _view->render(_renderer);

      if (_guiView) {
        _renderer->render(_guiView);
      }

      _renderer->endFrame();
      //_renderer->readPixels();
    }

    if (!refresh) {
      uint32_t interval = uint32_t(counter - stampPrev) / freq * 1000.0;
      if (interval < 14) {
        SDL_Delay(14 - interval);
      }
    }

    stampPrev = counter;
  }
}

void FTWin::gui(filament::Engine *, filament::View *)
{
  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
  ImGui::Begin("Panel");
  if (_fps > 0)
    ImGui::LabelText("FPS", "%8.3f", _fps);
  if (ImGui::Button("Add Cube")) {
    _view->scene()->addShape(0, 1);
  }
  if (ImGui::Button("Add Sphere")) {
    _view->scene()->addShape(1, 1);
  }

  static char text[256] = {0};
  if (ImGui::InputText("123", text, 256)) {
    printf("");
  }
  ImGui::End();
}

} // namespace fv
