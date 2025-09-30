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
class IndirectLight;
} // namespace filament

namespace fpc {
class PCDispatch;
}

namespace fv {

class Node;
class FTView;

class FTScene : public TScene {
public:
  FTScene();
  ~FTScene();

  filament::Engine *engine() { return _engine; }

  operator filament::Scene *() { return _scene; }
  filament::Scene *fila_scene() { return _scene; }

  const filament::Material *basic_material() { return _basic_material; }
  const filament::Material *default_material() { return _default_material; }

  void show_box(const tg::boundingbox &box);

  void set_environment(const std::string_view &prefix = "", bool filter = false);

public:

  void  showModel(int id, bool show);

  int   loadModel(const std::string &file, float sz);

  int   addShape(int);

  std::shared_ptr<Node> find_node(uint32_t rent);

public:

  void _add_node(const std::shared_ptr<Node> &node);

public:

  void initialize(filament::Engine *engine);

  void process(double delta);

private:
  bool _initialized = false;

  filament::Engine *_engine = nullptr;
  filament::Scene *_scene = nullptr;

  filament::Texture *_skybox_tex = nullptr;
  filament::Skybox *_skybox = nullptr;
  filament::Texture *_ibl_tex = nullptr;
  filament::IndirectLight *_ibl = nullptr;

  filament::Material const *_basic_material = nullptr;
  filament::Material const *_default_material = nullptr;
  filament::Material const *_legacy_material = nullptr;
  filament::Material const *_depth_material = nullptr;

  uint32_t _sunLight;

  uint32_t _point_count = 0;

  std::mutex _mutex;
  std::queue<std::function<void()>> _tasks;

  tsl::robin_map<uint32_t, std::shared_ptr<Node>> _nodes;
};

} // namespace fv
