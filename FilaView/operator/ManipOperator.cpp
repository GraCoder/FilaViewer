#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include "ManipOperator.h"
#include "TView.h"

#define THROW_EVENT SDL_USEREVENT + 1000

namespace fv {

ManipOperator::ManipOperator() 
{
  _upAxis = tg::vec3d(0, 1, 0);
  _rtAxis = tg::vec3d(1, 0, 0);
}

ManipOperator::~ManipOperator() {}

bool ManipOperator::process(double refTime)
{
  if (_throwTime > 0) {

    float f = 3 * (refTime - _refTime) / _throwTime;
    auto dx = _dx * f; auto dy = _dy * f;
    performLeft(0, dx, dy);
  }
  _refTime = refTime;
  return true;
}

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

bool ManipOperator::handle(TView *view, const SDL_Event *event)
{
  bool result = TOperator::handle(view, event);
  return result;
}

bool ManipOperator::mousePress(TView *view, const SDL_MouseButtonEvent &btn)
{
  {
  }
  if (btn.button == SDL_BUTTON_LEFT) {
  }
  _throwTime = 0;
  _dx = btn.x; _dy = btn.y;
  _throwStamp = btn.timestamp;
  return true;
}

bool ManipOperator::mouseRelease(TView *view, const SDL_MouseButtonEvent &btn)
{
  auto inv = btn.timestamp - _throwStamp;
  if (inv > 0) {
    _dx = btn.x - _dx; _dy = btn.y - _dy;
    float velocity = sqrt(_dx * _dx + _dy * _dy) / inv;
    if (velocity > 0.1) {
      _throwTime = inv;
      auto &vp = view->viewport();
      _dx /= vp.z(); _dy /= vp.w();
    }
  }

  return true;
}

bool ManipOperator::mouseMove(TView *view, const SDL_MouseMotionEvent &motion)
{
  auto &vp = view->viewport();
  float dx = motion.xrel, dy = motion.yrel;
  dx /= vp.z(); dy /= vp.w();
  if (motion.state & SDL_BUTTON_LMASK) {
    performLeft(view, dx, dy);
  } else if (motion.state & SDL_BUTTON_RMASK) {
    performRight(view, dx, dy);
  } else if (motion.state & SDL_BUTTON_MMASK) {
  }

  if (motion.state && (motion.timestamp - _throwStamp) > 50) {
    _dx = motion.x; _dy = motion.y;
    _throwStamp = motion.timestamp;
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
    _throwTime = 0;
    _distance = 100;
    _rotation = tg::quatd();
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
