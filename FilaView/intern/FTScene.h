#pragma once

#include <queue>
#include <functional>
#include <tmath.h>
#include <mutex>

#include "TScene.h"

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
class MeshAssimp;

class FTScene : public TScene {
public:

  FTScene();

  ~FTScene();

  filament::Engine* engine() { return _engine; }

  operator filament::Scene*() { return _scene; }
  filament::Scene* fila_scene() { return _scene; }

  const filament::Material *basic_material() { return _basic_material; }
  const filament::Material *default_material() { return _default_material; }

  void show_box(const tg::boundingbox &box);

  void add_test_scene();

  void load_model(const std::string &file, float sz);

public:

  void realize(filament::Engine *engine);

  void process(float delta);

private:

  void gui(filament::Engine *, filament::View *);

  void assimp_load(const std::string &file, float sz);

private:
  bool _realized = false;

  filament::Engine  *_engine = nullptr;
  filament::Scene   *_scene = nullptr;

  filament::Material const *_basic_material = nullptr;
  filament::Material const *_default_material = nullptr;
  filament::Material const *_transparent_material = nullptr;
  filament::Material const *_depth_material = nullptr;

  uint32_t _point_count = 0;

  std::mutex _mutex;
  std::queue<std::function<void()>> _tasks;

  std::unique_ptr<MeshAssimp> _assimp;
};

