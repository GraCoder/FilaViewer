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

FTView::FTView()
  : TView()
{
}

FTView::~FTView()
{
  if (_engine && _camera)
    _engine->destroy(_camera->getEntity());

  if (_engine && _view)
    _engine->destroy(_view);

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
             .orbitSpeed(0.005, 0.005)
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

  _camera_dirty = true;
}

void FTView::realize(filament::Engine *engine) 
{
  _engine = engine;

  _view = engine->createView();
  //_view->setPostProcessingEnabled(false);

  utils::Entity cam_ent;
  utils::EntityManager &em = utils::EntityManager::get();
  em.create(1, &cam_ent);
  _camera = engine->createCamera(cam_ent);
  _camera->setExposure(16.0f, 1 / 125.0f, 100.0f);
  _view->setCamera(_camera);

  reset_projection();

  if (_scene) {
    _scene->realize(engine);
    _view->setScene(*_scene);
  }
}

void FTView::set_scene(const std::shared_ptr<FTScene> &scene) 
{
  _scene = scene;
  if (_view && scene->fila_scene())
    _view->setScene(scene->fila_scene());
}

void FTView::process(float delta)
{ 
  update_camera();

  scene()->process(delta); 
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

  _camera->setCustomProjection(fmat, _near, _far);
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
  _camera_dirty = true;
}

void FTView::mouse_move(int x, int y)
{
  if (_grabing) {
    _manip->grabUpdate(x, y);
    _camera_dirty = true;
  } 
}

void FTView::mouse_wheel(int x, int y, float deltay)
{
  if (_manip) {
    _manip->scroll(x, y, deltay);
  }

  _camera_dirty = true;
}

void FTView::key_down(SDL_Scancode scancode) { _camera_dirty = true; }

void FTView::key_up(SDL_Scancode scancode)
{
  if (scancode == SDL_SCANCODE_SPACE) {
    _manip->jumpToBookmark(_manip->getHomeBookmark());
  }
  _camera_dirty = true;
}

void FTView::set_viewport(int x, int y, uint32_t width, uint32_t height)
{
  //auto aspectRatio = double(width) / height;
  //_camera->setScaling({1.0 / aspectRatio, 1.0});

  _view->setViewport({x, y, width, height});

  if(_manip)
    _manip->setViewport(width, height);

  reset_projection();
}

void FTView::update_camera()
{
  if (!_camera_dirty)
    return;

  _camera_dirty = false;

  math::float3 eye, center, up;
  _manip->getLookAt(&eye, &center, &up);
  _camera->lookAt(eye, center, up);

  if (scene())
    scene()->dispatch();
}
