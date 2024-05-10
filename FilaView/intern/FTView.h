#pragma once

#include "TView.h"

#include <functional>
#include <SDL2/SDL_scancode.h>

namespace filament {
class View;
class Camera;
class Engine;
} // namespace filament

class FTScene;

class FTView : public TView {
  friend class TView;
  friend class FTWin;
public:

  FTView();
  ~FTView();

  filament::Engine *engine() { return _engine; }

  operator filament::View*() { return _view; }
  filament::View *fila_view() { return _view; }

  void set_pivot(const tg::vec3d &pos, double dis = 100);

public:

  void realize(filament::Engine *engine);

  const std::shared_ptr<FTScene> &scene() { return _scene; }
  void set_scene(const std::shared_ptr<FTScene> &scene);

  void process(float delta);

public:

  void reset_projection();

  typedef std::function<void(filament::Engine*, filament::View*)> GuiCallback;
  void set_gui_callback(const GuiCallback &cb) { _gui_callback = cb; }

  void clean();

protected:

  void mouse_down(int button, int x, int y);
  void mouse_up(int x, int y);
  void mouse_move(int x, int y);
  void mouse_wheel(int x, int y, float deltay);
  void key_down(SDL_Scancode scancode);
  void key_up(SDL_Scancode scancode);

  void set_viewport(int x, int y, uint32_t w, uint32_t h);
  void update_camera();
    
  GuiCallback gui_callback() { return _gui_callback; };

private:

  filament::Engine    *_engine = nullptr;
  filament::View      *_view = nullptr;
  filament::Camera    *_camera = nullptr;

  Manipulator *_manip = nullptr;

  bool    _grabing = false;

  GuiCallback _gui_callback;

  std::shared_ptr<FTScene> _scene = nullptr;
};
