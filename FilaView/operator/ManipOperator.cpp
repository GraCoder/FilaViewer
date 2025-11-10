#include <SDL2/SDL_events.h>
#include "ManipOperator.h"
#include "TView.h"

#define RefreshView if(_view) _view->dirtyCamera();

namespace fv {

ManipOperator::ManipOperator() 
{
  _upAxis = tg::vec3d(0, 1, 0);
  _rtAxis = tg::vec3d(1, 0, 0);
}

ManipOperator::~ManipOperator() {}

void ManipOperator::getLookAt(tg::vec3d &eye, tg::vec3d &target, tg::vec3d &up)
{
  if (_upAxis && _rtAxis) {
    target = _target;
    up = _rotation * _upAxis.value();
    eye = _target + _rotation * tg::vec3d(0, 0, _distance);
  }
}

void ManipOperator::setPivot(const tg::vec3d &pos, double dis) {}

void ManipOperator::setViewport(int w, int h) {}

bool ManipOperator::mousePress(TView *view, const SDL_MouseButtonEvent &btn)
{
  _ms_t0.x = _ms_t1.x = btn.x;
  _ms_t1.y = _ms_t1.y = btn.y;

  if (btn.button == SDL_BUTTON_LEFT) {
    _ms_t0.button = SDL_BUTTON_LEFT;
  }
  return true;
}

bool ManipOperator::mouseRelease(TView *view, const SDL_MouseButtonEvent &btn)
{
  _ms_t0.x = btn.x;
  _ms_t0.y = btn.y;

  return true;
}

bool ManipOperator::mouseMove(TView *view, const SDL_MouseMotionEvent &motion)
{
  _ms_t1 = _ms_t0;
  _ms_t0.x = motion.x;
  _ms_t0.y = motion.y;

  auto &vp = view->viewport();
  float dx = _ms_t0.x - _ms_t1.x, dy = _ms_t0.y - _ms_t1.y;
  dx = dx / vp.z(), dy = dy / vp.w();

  if (motion.state & SDL_BUTTON_LMASK) {
    _ms_t0.button = SDL_BUTTON_LEFT;
    performLeft(view, dx, dy); 
  } else if (motion.state & SDL_BUTTON_RMASK) {
    _ms_t0.button = SDL_BUTTON_RIGHT;
    performRight(view, dx, dy);
  } else if (motion.state & SDL_BUTTON_MMASK) {
    _ms_t0.button = SDL_BUTTON_MIDDLE;
  }
  return true;
}

bool ManipOperator::mouseWheel(TView *view, const SDL_MouseWheelEvent &wheel)
{
  float scale = 1.f + wheel.preciseY * 0.2;
  scale = tg::clamp(scale, 0.1f, 1e6f);
  _distance *= scale;
  return true;
}

bool ManipOperator::keyPress(TView *view, const SDL_KeyboardEvent &key)
{
  if (key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
    //_close = true;
  } else if (key.keysym.scancode == SDL_SCANCODE_LCTRL) {
    //_view->filaView()->set_manip_factor(10.0);
  } else if (key.keysym.scancode == SDL_SCANCODE_LSHIFT) {
    // view()->set_manip_factor(5.0);
  }
  return true;
}

bool ManipOperator::keyRelease(TView *view, const SDL_KeyboardEvent &key)
{
  if (key.keysym.scancode == SDL_SCANCODE_SPACE) {
  }
  return true;
}

void ManipOperator::performLeft(TView *view, float dx, float dy) 
{
  if(_upAxis && _rtAxis) {
    tg::quatd qx = tg::quatd::rotate(-dx, *_upAxis);
    auto right = qx * _rotation * *_rtAxis;
    tg::quatd qy = tg::quatd::rotate(-dy, right);
  
    _rotation = qy * qx * _rotation;
  }

}

void ManipOperator::performRight(TView *view, float dx, float dy)
{
  tg::vec3d eye, target, up;
  getLookAt(eye, target, up);

  auto dir = normalize(target - eye);
  auto rt = tg::cross(dir, up);

  double fovy, aspect, near, far;
  if(tg::get_perspective(view->projectionMatrix(), fovy, aspect, near, far)) {
    double fv = tan(M_PI * fovy / 2.0 / 180) * _distance * 2;
    tg::vec3d oft = rt * -dx * fv * aspect + up * dy * fv;
    _target += oft;
  }
}


} // namespace fv
