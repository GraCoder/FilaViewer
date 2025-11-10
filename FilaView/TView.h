#pragma once

#include <optional>
#include <memory>
#include "tmath.h"
#include "IView.h"

namespace fv {

class TScene;

class TView : public IView {
public:
  static std::shared_ptr<TView> create();

  ~TView();

  const tg::vec4i &viewport() { return _viewport; }
  tg::mat4d projectionMatrix();

  void zoomBox(const tg::boundingbox &box);

  std::optional<tg::vec3d> getPosition(int x, int y);

public:
  void dirtyCamera() { _dirtyCamera = true; }

protected:

  TView();

protected:
  bool _dirtyCamera = true;
  float _near = 0.4, _far = 2000;

  tg::vec4i _viewport;
};

} // namespace fv