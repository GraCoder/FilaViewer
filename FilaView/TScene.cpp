#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Frustum.h>

#include "nlohmann/json.hpp"

#include "TDef.h"
#include "TView.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"

#include "node/mesh/Cube.h"
#include "node/mesh/Sphere.h"
#include "node/ShapeNode.h"

#include "TScene.h"

namespace fv {

FT_DOWNCAST(TScene)
FT_DOWNCAST(TView)

std::shared_ptr<TScene> TScene::create()
{
  auto scene = std::make_shared<FTScene>();
  return scene;
}

TScene::TScene() = default;

TScene::~TScene()
{
}

void TScene::registerHandlers(std::unordered_map<uint64_t, std::function<int(const std::string_view &)>> &handlers) 
{
  handlers[std::hash<std::string>{}("AddCube")] = [this](const std::string_view &cmd) { return addShape(cmd); };
  handlers[std::hash<std::string>{}("AddSphere")] = [this](const std::string_view &cmd) { return addShape(cmd); };
}

void TScene::addNode(const std::shared_ptr<Node> &node)
{
  downcast(this)->_addNode(node);
}

int TScene::addShape(const std::string_view &cmd)
{
  auto js = nlohmann::json::parse(cmd);

  auto op = js["Name"].get<std::string_view>();
  auto rad = js["Rad"].get<float>();

  if (op == "AddCube") {
    return addShape(0, rad);
  } else if (op == "AddSphere") {
    return addShape(1, rad);
  }
  return -1;
}

int TScene::addShape(int priType, float rad)
{
  using namespace filament;

  std::shared_ptr<ShapeNode> node;
  switch (priType) {
  case 0:
    node = std::make_shared<ShapeNode>(std::make_unique<Cube>(math::float3{0, 0, 0}, math::float3{5, 5, 5}));
    break;
  case 1:
    node = std::make_shared<ShapeNode>(std::make_unique<Sphere>(math::float3{0, 0, 0}, 5));
    break;
  }
  if (!node)
    return -1;

  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push(std::bind([this, node]() { addNode(node); }));
  return node->id();
}

} // namespace fv
