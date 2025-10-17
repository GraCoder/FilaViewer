#include "filament/View.h"
#include "intern/FTScene.h"
#include "intern/FTView.h"
#include "node/Node.h"
#include "PickOperator.h"

namespace fv {

PickOperator::PickOperator(TView *view)
  : TOperator()
  , _view(view)
{
}

PickOperator::~PickOperator() {}

bool PickOperator::mouse_press(const SDL_MouseButtonEvent &btn)
{
  _pos = tg::vec2(btn.x, btn.y);
  return false;
}

bool PickOperator::mouse_release(const SDL_MouseButtonEvent &btn)
{
  if (abs(_pos.x() - btn.x) > 3 || abs(_pos.y() - btn.y) > 3)
    return false;

  auto v = static_cast<FTView *>(_view)->fila_view();
  v->pick(btn.x, btn.y, [this](filament::View::PickingQueryResult const &result) {
    auto s = static_cast<FTView *>(_view)->scene();
    auto node = s->findNode(result.renderable.getId());
    if (_fun) {
      if (node)
        _fun(node->id());
      else
        _fun(0);
    }
  });
  return false;
}

} // namespace fv
