#include <filament/View.h>
#include <filament/Viewport.h>
#include "FTView.h"
#include "ManipOperator.h"

using namespace filament::camutils;

ManipOperator::ManipOperator(FTView *view)
  : _view(view)
{
}

ManipOperator::~ManipOperator()
{
  if (_manip) {
    delete _manip;
    _manip = nullptr;
  }
}

void ManipOperator::set_pivot(const tg::vec3d &pos, double dis)
{
  using namespace filament::camutils;

  auto &vp = _view->fila_view()->getViewport();

  _manip = Manipulator::Builder()
             .targetPosition(pos.x(), pos.y(), pos.z())
             .orbitHomePosition(pos.x(), pos.y(), pos.z() + dis)
             .viewport(vp.width, vp.height)
             .orbitSpeed(0.004, 0.004)
             //
             //.fovDirection(Fov::VERTICAL)
             //.fovDegrees(_camera->getFieldOfViewInDegrees(Camera::Fov::VERTICAL))
             //.farPlane(_far)
             //.mapExtent(100, 100)
             //.mapMinDistance(_near)
             //.groundPlane(0, 1, 0, 0)
             //.upVector(0, 0, -1)
             //
             .zoomSpeed(5)
             .flightMoveDamping(15.0)
             .build(filament::camutils::Mode::ORBIT);
}

void ManipOperator::set_viewport(int w, int h) 
{
  _width = w;
  _height = h;
  _manip->setViewport(w, h);
}

bool ManipOperator::mouse_press(const SDL_MouseButtonEvent &btn)
{
  _grabing = true;
  _manip->grabBegin(btn.x, _height - btn.y, btn.button == SDL_BUTTON_RIGHT);

  if (btn.button == SDL_BUTTON_LEFT)
    _view->get_pos(btn.x, btn.y);

  return true;

}

bool ManipOperator::mouse_release(const SDL_MouseButtonEvent &btn)
{
  _grabing = false;
  _manip->grabEnd();
  _view->dirty_camera();

  return true;
}

bool ManipOperator::mouse_wheel(const SDL_MouseWheelEvent &wheel)
{
  if (_manip) {
    _manip->scroll(wheel.x, _height - wheel.y, wheel.preciseY);
  }

  _view->dirty_camera();

  return true;
}

bool ManipOperator::mouse_move(const SDL_MouseMotionEvent &motion)
{
  if (_grabing) {
    _manip->grabUpdate(motion.x, _height - motion.y);
    _view->dirty_camera();
  }
  return true;
}

bool ManipOperator::key_press(const SDL_KeyboardEvent &key)
{
  if (key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
    //_close = true;
  } else if (key.keysym.scancode == SDL_SCANCODE_LCTRL) {
    //_view->fila_view()->set_manip_factor(10.0);
  } else if (key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
    //view()->set_manip_factor(5.0);
  }
  _view->dirty_camera();
  return true;
}

bool ManipOperator::key_release(const SDL_KeyboardEvent &key)
{
  if (key.keysym.scancode == SDL_SCANCODE_SPACE) {
    _manip->jumpToBookmark(_manip->getHomeBookmark());
  }
  _view->dirty_camera();
  return true;
}

void ManipOperator::get_lookat(filament::math::float3 &eye, filament::math::float3 &target, filament::math::float3 &up) 
{
  _manip->getLookAt(&eye, &target, &up);
}
