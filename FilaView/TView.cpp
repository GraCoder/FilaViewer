#include "TView.h"

#include <filament/Camera.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include "TDef.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"
#include "intern/FTWin.h"

using namespace filament;

namespace fv {

FT_DOWNCAST(TView)

TView::TView() {}

std::shared_ptr<TView> TView::create()
{
  return std::make_shared<FTView>();
}

TView::~TView() {}

// TView::Manipulator *TView::manip()
//{
//   return downcast(this)->_manip;
// }

tg::mat4d TView::projectionMatrix()
{
  tg::mat4d m;
  m.set(downcast(this)->_camera->getProjectionMatrix().asArray());
  return m;
}

void TView::zoomBox(const tg::boundingbox &box)
{
  auto m = downcast(this)->_camera->getProjectionMatrix();
  float l = m[1][1] * box.radius();
}

std::optional<tg::vec3d> TView::getPosition(int x, int y)
{
  auto fm = downcast(this)->_camera->getProjectionMatrix();
  fm = fm * downcast(this)->_camera->getViewMatrix();
  auto &vp = downcast(this)->_view->getViewport();
  double fx = double(x - vp.left) / vp.width * 2.0 - 1;
  double fy = double(y - vp.bottom) / vp.height * 2.0 - 1;
  int pix_scope = 2.0;
  double dx = 2.0 / vp.width * pix_scope, dy = 2.0 / vp.height * pix_scope;
  tg::mat4d m;
  m.set(fm.asArray());
  m = m.transpose();

  tg::vec4d planes[5];
  planes[0] = m * tg::vec4d(1, 0, 0, -fx + dx);
  planes[1] = m * tg::vec4d(-1, 0, 0, fx + dx);
  planes[2] = m * tg::vec4d(0, 1, 0, -fy + dy);
  planes[3] = m * tg::vec4d(0, -1, 0, fy + dy);
  planes[4] = m * tg::vec4d(0, 0, 1, 0);

  auto cam_pos = downcast(this)->_camera->getPosition();
  for (int i = 0; i < 5; i++) {
    double len = tg::length(tg::vec3d(planes[i].data()));
    planes[i] = planes[i] / len;
  }

  return std::optional<tg::vec3d>();
}

} // namespace fv
