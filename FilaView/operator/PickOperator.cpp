#include "filament/View.h"
#include "intern/FTScene.h"
#include "intern/FTView.h"
#include "node/Node.h"
#include "PickOperator.h"

PickOperator::PickOperator(TView *view)
  : TOperator()
  , _view(view)
{
}

PickOperator::~PickOperator() 
{
}

bool PickOperator::mouse_press(const SDL_MouseButtonEvent &btn)
{
  _pos = tg::vec2(btn.x, btn.y);
  return false;
}

bool PickOperator::mouse_release(const SDL_MouseButtonEvent &btn)
{
  if (abs(_pos.x() - btn.x) > 2 || abs(_pos.y() - btn.y) > 2)
    return false;

  auto v = static_cast<FTView *>(_view)->fila_view();
  v->pick(btn.x, btn.y, [this](filament::View::PickingQueryResult const &result){ 
    auto s = static_cast<FTView *>(_view)->scene(); 
    auto node = s->find_node(result.renderable.getId());
    if (_fun) _fun(node->id());
  });
  return false;
}
