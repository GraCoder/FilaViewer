#pragma once

#include <queue>
#include <functional>
#include <tmath.h>
#include <mutex>

#include "TScene.h"
#include "tsl/robin_map.h"

namespace filament {
class View;
class Scene;
class Engine;
class Skybox;
class Texture;
class Material;
} // namespace filament

namespace fpc{
class PCDispatch;
}

class FTView;

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

  void set_environment(const std::string &img = "", bool filter = false);

public:

  int     load_model(const std::string &file, float sz);

  void    show_model(int id, bool show);

  int     add_shape(int);

public:

  void _add_node(const std::shared_ptr<Node> &node);

public:

  void realize(filament::Engine *engine);

  void process(double delta);

private:

  void gui(filament::Engine *, filament::View *);

private:
  bool _realized = false;

  filament::Engine  *_engine = nullptr;
  filament::Scene   *_scene = nullptr;

  filament::Texture *_skybox_tex = nullptr;
  filament::Skybox  *_skybox = nullptr;

  filament::Material const *_basic_material = nullptr;
  filament::Material const *_default_material = nullptr;
  filament::Material const *_legacy_material = nullptr;
  filament::Material const *_depth_material = nullptr;

  uint32_t _point_count = 0;

  std::mutex _mutex;
  std::queue<std::function<void()>> _tasks;

  tsl::robin_map<uint32_t, std::shared_ptr<Node>> _nodes;
};

