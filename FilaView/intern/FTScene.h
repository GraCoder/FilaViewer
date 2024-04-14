#pragma once

#include "TScene.h"
#include <pcd/tmath.h>

namespace filament {
class View;
class Scene;
class Engine;
class Material;
} // namespace filament

namespace fpc{
class PCDispatch;
}

class FTView;

class FTScene : public TScene {
public:

  FTScene(FTView *view);

  ~FTScene();

  FTView *view() { return _view; }

  filament::Engine &engine() { return _engine; }

  filament::Scene *scene() { return _scene; }

  const filament::Material *basic_material() { return _basic_material; }
  const filament::Material *default_material() { return _default_material; }

  void show_box(const tg::boundingbox &box);

private:

  void gui(filament::Engine *, filament::View *);

private:

  FTView *_view = nullptr;

  filament::Engine &_engine;

  filament::Scene *_scene = nullptr;

  filament::Material const *_basic_material = nullptr;
  filament::Material const *_default_material = nullptr;
  filament::Material const *_transparent_material = nullptr;
  filament::Material const *_depth_material = nullptr;

  uint32_t _point_count = 0;
};

