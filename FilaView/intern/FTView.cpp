#include "FTView.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/DebugRegistry.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include "FTScene.h"
#include "Operator/ManipOperator.h"

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

void FTView::showEntity(int id, bool show) 
{
  static_cast<FTScene *>(_scene.get())->showEntity(id, show);
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

  resetProjection();

  if (_scene) {
    _scene->initialize(engine);
    _view->setScene(*_scene);
  }
}

void FTView::setScene(const std::shared_ptr<FTScene> &scene)
{
  _scene = scene;
  if (_view && scene->filaScene())
    _view->setScene(scene->filaScene());
}

void FTView::process(double delta)
{
  scene()->process(delta);
}

void FTView::resetProjection()
{
  if (_view == nullptr)
    return;
  static constexpr const float SENSOR_SIZE = 0.024f; // 24mm
  auto &vp = _view->getViewport();
  if (vp.width == 0 || vp.height == 0)
    return;

  double const h = (0.5 * _near) * ((SENSOR_SIZE * 1000.0) / 35.0);
  double const w = h * vp.width / vp.height;

  auto mat = tg::frustum(-w, w, -h, h, _near, _far);
  filament::math::mat4 fmat;
  memcpy(&fmat, &mat, sizeof(tg::mat4d));

  _camera->setCustomProjection(fmat, _near, _far);
}

void FTView::release()
{
}

void FTView::setViewport(int x, int y, uint32_t width, uint32_t height)
{
  // auto aspectRatio = double(width) / height;
  //_camera->setScaling({1.0 / aspectRatio, 1.0});

  _viewport.set(x, y, width, height);

  if (_view)
    _view->setViewport({x, y, width, height});

  resetProjection();
}

} // namespace fv
