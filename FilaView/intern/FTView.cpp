#include "FTView.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <camutils/Manipulator.h>

#include "FTScene.h"

using namespace filament;
using namespace filament::camutils;

FTView::FTView(filament::Engine &engine)
  : TView()
  , _engine(engine)
{
  _view = engine.createView();

  utils::Entity camEnt;
  utils::EntityManager &em = utils::EntityManager::get();
  em.create(1, &camEnt);
  _cam = engine.createCamera(camEnt);
  _cam->setExposure(16.0f, 1 / 125.0f, 100.0f);
  _view->setCamera(_cam);

  reset_projection();

  set_pivot({0, 0, 0}, 15);

  _scene = TScene::create(this);
}

FTView::~FTView()
{
  if (_cam)
    _engine.destroy(_cam->getEntity());

  if (_view)
    _engine.destroy(_view);

  if (_manip)
    delete _manip;
}

void FTView::set_pivot(const tg::vec3d &pos, double dis) 
{
  if (_manip)
    delete _manip;

  auto &vp = _view->getViewport();

  _manip = Manipulator::Builder()
             .targetPosition(pos.x(), pos.y(), pos.z())
             .orbitHomePosition(pos.x(), pos.y(), pos.z() + dis)
             .viewport(vp.width, vp.height)
             //
             //.fovDirection(Fov::VERTICAL)
             //.fovDegrees(_cam->getFieldOfViewInDegrees(Camera::Fov::VERTICAL))
             //.farPlane(_far)
             //.mapExtent(100, 100)
             //.mapMinDistance(_near)
             //.groundPlane(0, 1, 0, 0)
             //.upVector(0, 0, -1)
             //
             .zoomSpeed(5)
             .flightMoveDamping(15.0)
             .build(filament::camutils::Mode::ORBIT);

  _cam_dirty = true;
}

void FTView::reset_projection()
{
  static constexpr const float SENSOR_SIZE = 0.024f; // 24mm
  auto &vp = _view->getViewport();
  if (vp.width == 0 || vp.height == 0)
    return;

  double const h = (0.5 * _near) * ((SENSOR_SIZE * 1000.0) / 35.0);
  double const w = h * vp.width / vp.height;

  auto mat = tg::frustum<double>(-w, w, -h, h, _near, _far);
  filament::math::mat4 fmat;
  memcpy(&fmat, &mat, sizeof(tg::mat4d));

  _cam->setCustomProjection(fmat, _near, _far);
}

void FTView::clean() { _scene->clean(); }

void FTView::mouse_down(int button, int x, int y)
{
  _grabing = true;
  _manip->grabBegin(x, y, button == 3);

  if(button == 1)
    get_pos(x, y);
}

void FTView::mouse_up(int x, int y)
{
  _grabing = false;
  _manip->grabEnd();
  _cam_dirty = true;
}

void FTView::mouse_move(int x, int y)
{
  if (_grabing) {
    _manip->grabUpdate(x, y);
    _cam_dirty = true;
  } 
}

void FTView::mouse_wheel(int x, int y, float deltay)
{
  if (_manip) {
    _manip->scroll(x, y, deltay);
  }

  _cam_dirty = true;
}

void FTView::key_down(SDL_Scancode scancode) { _cam_dirty = true; }

void FTView::key_up(SDL_Scancode scancode)
{
  if (scancode == SDL_SCANCODE_SPACE) {
    _manip->jumpToBookmark(_manip->getHomeBookmark());
  }
  _cam_dirty = true;
}

void FTView::set_viewport(int x, int y, uint32_t width, uint32_t height)
{
  //auto aspectRatio = double(width) / height;
  //_cam->setScaling({1.0 / aspectRatio, 1.0});

  _view->setViewport({x, y, width, height});

  _manip->setViewport(width, height);

  reset_projection();
}

void FTView::update_camera()
{
  if (!_cam_dirty)
    return;

  _cam_dirty = false;

  math::float3 eye, center, up;
  _manip->getLookAt(&eye, &center, &up);
  _cam->lookAt(eye, center, up);

  if(scene())
    scene()->dispatch();
}
