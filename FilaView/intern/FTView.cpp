#include "FTView.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/DebugRegistry.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include "FTScene.h"
#include "ManipOperator.h"

using namespace filament;

namespace fv {

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
}

void FTView::realize(filament::Engine *engine)
{
  _engine = engine;
  _view = engine->createView();

  {
    _view->setBloomOptions(filament::BloomOptions{.enabled = false});
    _view->setAmbientOcclusionOptions(filament::AmbientOcclusionOptions{.enabled = false});

    _view->setDithering(filament::Dithering::NONE);
    _view->setTemporalAntiAliasingOptions(filament::TemporalAntiAliasingOptions{.enabled = false});

    _view->setMultiSampleAntiAliasingOptions(filament::MultiSampleAntiAliasingOptions{.enabled = false});

    _view->setAntiAliasing(filament::AntiAliasing::NONE);

    _view->setPostProcessingEnabled(true);
  }

  _view->setShadowingEnabled(false);

  utils::Entity cam_ent;
  utils::EntityManager &em = utils::EntityManager::get();
  em.create(1, &cam_ent);
  _camera = engine->createCamera(cam_ent);
  _camera->setExposure(16.0f, 1 / 125.0f, 100.0f);
  _view->setCamera(_camera);

  reset_projection();

  if (_scene) {
    _scene->initialize(engine);
    _view->setScene(*_scene);
  }

  _manip = std::make_shared<ManipOperator>(this);
}

void FTView::set_scene(const std::shared_ptr<FTScene> &scene)
{
  _scene = scene;
  if (_view && scene->fila_scene())
    _view->setScene(scene->fila_scene());
}

void FTView::process(double delta)
{
  update_camera();

  scene()->process(delta);
}

void FTView::reset_projection()
{
  if (_view == nullptr)
    return;
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

void FTView::release()
{
}

void FTView::set_viewport(int x, int y, uint32_t width, uint32_t height)
{
  // auto aspectRatio = double(width) / height;
  //_camera->setScaling({1.0 / aspectRatio, 1.0});

  if (_view)
    _view->setViewport({x, y, width, height});

  if (_manip)
    _manip->set_viewport(width, height);

  reset_projection();
}

void FTView::update_camera()
{
  if (!_camera)
    return;

  if (!_camera_dirty)
    return;

  _camera_dirty = false;

  math::float3 eye, center, up;
  _manip->get_lookat(eye, center, up);
  _camera->lookAt(eye, center, up);
}

} // namespace fv
