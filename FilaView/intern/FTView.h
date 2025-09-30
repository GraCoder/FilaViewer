#pragma once

#include "TView.h"

#include <functional>

namespace filament {
class View;
class Camera;
class Engine;
} // namespace filament

namespace fv {

class FTScene;
class ManipOperator;

class FTView : public TView {
  friend class TView;
  friend class FTWin;

public:
  FTView();
  ~FTView();

  filament::Engine *engine() { return _engine; }

  operator filament::View *() { return _view; }
  filament::View *fila_view() { return _view; }

public:

  void realize(filament::Engine *engine);

  const std::shared_ptr<ManipOperator> &manip() { return _manip; }

  const std::shared_ptr<FTScene> &scene() { return _scene; }
  void set_scene(const std::shared_ptr<FTScene> &scene);

  void process(double delta);

public:
  void dirty_camera() { _camera_dirty = true; }
  void reset_projection();

  void release();

protected:

  void set_viewport(int x, int y, uint32_t w, uint32_t h);
  void update_camera();

private:

  filament::Engine *_engine = nullptr;
  filament::View *_view = nullptr;
  filament::Camera *_camera = nullptr;

  std::shared_ptr<FTScene> _scene = nullptr;
  std::shared_ptr<ManipOperator> _manip = nullptr;
};

} // namespace fv
