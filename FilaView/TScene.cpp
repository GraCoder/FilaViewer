#include "TScene.h"

#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Frustum.h>

#include "TDef.h"
#include "TView.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"

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

void TScene::addNode(const std::shared_ptr<Node> &node)
{
  downcast(this)->_add_node(node);
}

} // namespace fv
