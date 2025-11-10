#pragma once

#include "TView.h"

#include <functional>

namespace filament {
class View;
class Camera;
class Engine;
} // namespace filament

namespace fv {

class FTScene;

class FTView : public TView {
  friend class TView;
  friend class FTWin;

public:
  FTView();
  ~FTView();

  filament::Engine *engine() { return _engine; }

  operator filament::View *() { return _view; }
  filament::View *view() { return _view; }

  void showEntity(int id, bool show) override;

public:

  void realize(filament::Engine *engine);

  const std::shared_ptr<FTScene> &scene() { return _scene; }
  void setScene(const std::shared_ptr<FTScene> &scene);

  void process(double delta);

public:
  void resetProjection();

  void release();

protected:

  void setViewport(int x, int y, uint32_t w, uint32_t h);

private:

  filament::Engine *_engine = nullptr;
  filament::View *_view = nullptr;
  filament::Camera *_camera = nullptr;

  std::shared_ptr<FTScene> _scene = nullptr;
};

} // namespace fv
