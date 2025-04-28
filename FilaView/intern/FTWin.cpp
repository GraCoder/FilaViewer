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
  FTWin* win = new FTWin(static_cast<FTWin *>(s));
  if(!with_border)
    win->set_flags((uint32_t)TWin::en_Frameless);
  return win;
}

void IWin::destroy(IWin *win) 
{
  FTWin *w = static_cast<FTWin *>(win);
  delete w;
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
  if (_gui) {
    delete _gui;
    _gui = nullptr;
  }

  if(_gui_view) {
    _engine->destroy(_gui_view);
    _gui_view = nullptr;
  }

  if (_swapchain) {
    _engine->destroy(_swapchain);
    _swapchain = nullptr;
  }
  if (_renderer) {
    _engine->destroy(_renderer);
    _renderer = nullptr;
  }

  _view.reset();

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

  // SDL_GL_GetDrawableSize(_window, &_width, &_height);
  uint32_t width = _width, height = _height;

  int virtualWidth, virtualHeight;
  SDL_GetWindowSize(_window, &virtualWidth, &virtualHeight);
  dpiScaleX = (float)width / virtualWidth;
  dpiScaleY = (float)height / virtualHeight;

  _view->manip()->set_pivot({0, 0, 0}, 15);
  _view->set_viewport(0, 0, width, height);
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

void FTWin::create_window() 
{
  const int x = SDL_WINDOWPOS_CENTERED;
  const int y = SDL_WINDOWPOS_CENTERED;

  uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
  if(_flags & en_Frameless)
    flags |= SDL_WINDOW_BORDERLESS;
  auto win = SDL_CreateWindow("", x, y, _width, _height, flags);
  SDL_SetWindowResizable(win, SDL_TRUE);
  SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

  _window = win;
}

void FTWin::realize_context() 
{
  if (_realized)
    return;

  _realized = true;

  create_engine(); 

  _view->realize(_engine);

  configure_cameras();

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
    setup_gui(); 
}

void FTWin::create_engine()
{
  using namespace filament;

  backend::Backend backend = Engine::Backend::VULKAN;
  //Engine::Config engineConfig = {};

  _engine = Engine::Builder().backend(backend) /*.config(&engineConfig)*/.build();
  if (_engine)
    return;

  backend = Engine::Backend::OPENGL;
  _engine = Engine::Builder().backend(backend) /*.config(&engineConfig)*/.build();
}

#define OperIter for(auto iter = _operators.rbegin(); iter != _operators.rend(); iter++)(*iter) 
void FTWin::poll_events() 
{
  uint32_t freq = SDL_GetPerformanceFrequency() / 1000;
  uint64_t stamp = SDL_GetPerformanceCounter(), prev_time = stamp;

  static constexpr int mk[4] = {0, 0, 1, 2};
  constexpr int max_event = 16;
  SDL_Event events[max_event];

  while (!_close) {
    if (!UTILS_HAS_THREADING)
      _engine->execute();

    int ev_count = 0;
    bool immediate = false;

    while (ev_count < max_event && SDL_PollEvent(&events[ev_count++]) != 0) {}
    for (int i = 0; i < ev_count; i++) {
      const SDL_Event &event = events[i];
      switch (event.type) {
      case SDL_QUIT: {
        view()->release();
        _close = true;
        break;
      }
      case SDL_KEYDOWN:
        OperIter->key_press(event.key);
        break;
      case SDL_KEYUP:
        OperIter->key_release(event.key);
        break;
      case SDL_MOUSEWHEEL: {
        OperIter->mouse_wheel(event.wheel);
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
        OperIter->mouse_press(event.button);
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
        OperIter->mouse_release(event.button);
        break;
      }
      case SDL_MOUSEMOTION: {
        if (_gui) {
          auto &io = ImGui::GetIO();
          io.AddMousePosEvent(event.motion.x, event.motion.y);
          if (io.WantCaptureMouse)
            break;
        }
        OperIter->mouse_move(event.motion);
        break;
      }
      case SDL_DROPFILE:
        // SDL_free(event.drop.file);
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        //case SDL_WINDOWEVENT_RESIZED: 
        {
          immediate = true;
          uint32_t w = event.window.data1;
          uint32_t h = event.window.data2;
          resize(w, h);
          if (_gui) {
            _gui->setDisplaySize(w, h);
          }
          if (_gui_view) {
            _gui_view->setViewport({0, 0, w, h});
          }
          break;
        }
        case SDL_WINDOWEVENT_EXPOSED:
        case SDL_WINDOWEVENT_SHOWN: {
          if (!_realized)
            realize_context();
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
      
      if (_gui_view) {
        _renderer->render(_gui_view);
      }

      _renderer->endFrame();
      //_renderer->readPixels();
    }
  }
}

void FTWin::gui(filament::Engine *, filament::View *)
{
  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
  ImGui::Begin("Panel");
  if(ImGui::Button("Add Cube")) 
    _view->scene()->add_shape(0);
  if(ImGui::Button("Add Sphere"))
    _view->scene()->add_shape(1);
#ifdef POINT_CLOUD_SUPPORT

  if (ImGui::Button("Add PC")) {
    auto pc = std::make_shared<fpc::PCNode>();
    if (pc->load_file("D:\\07_Temp\\tiny\\test.tpcd")) {
      add_pc(pc);
      auto ab = pc->get_aabb();
      view()->set_pivot(ab.center(), 200);
    }
  }
  if (ImGui::Button("Calculate Point")) {
    _point_count = 0;
    for (auto iter : _pcs)
      _point_count += iter.second->point_count();
  }
  ImGui::Text("Point Count: %d", _point_count);
#endif
  ImGui::End();
}

