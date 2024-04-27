#include "TView.h"

#include <filament/Camera.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include "TDef.h"
#include "TScene.h"
#include "TWin.h"

#include "intern/FTView.h"
#include "intern/FTWin.h"

using namespace filament;

FT_DOWNCAST(TView)

TView::TView() {}

std::shared_ptr<TView> TView::create(TWin *win)
{
  auto &engine = *static_cast<FTWin *>(win)->engine();
  return std::make_shared<FTView>(engine);
}

TView::~TView() {
}


TView::Manipulator *TView::manip()
{
  return downcast(this)->_manip;
}

void TView::set_manip_factor(float f)
{
  //_manip->
}

void TView::process(float delta) 
{ 
  downcast(this)->update_camera();

  scene()->process(delta); 
}

void TView::zoom_box(const tg::boundingbox &box)
{
  using namespace filament::camutils;
  auto m = downcast(this)->_cam->getProjectionMatrix();
  float l = m[1][1] * box.radius();
}

std::optional<tg::vec3d> TView::get_pos(int x, int y)
{
  auto fm = downcast(this)->_cam->getProjectionMatrix();
  fm = fm * downcast(this)->_cam->getViewMatrix();
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

  auto cam_pos = downcast(this)->_cam->getPosition();
  for (int i = 0; i < 5; i++) {
    double len = tg::length(tg::vec3d(planes[i].data()));
    planes[i] = planes[i] / len;
  }

  _scene->get_pos(planes);

  return std::optional<tg::vec3d>();
}

