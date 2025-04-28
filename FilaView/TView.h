#pragma once

#include <optional>
#include <memory>
#include "tmath.h"
#include "IView.h"

class TWin;
class TScene;

class TView : public IView {
public:

  static std::shared_ptr<TView> create(TWin *win);

  ~TView();

  void zoom_box(const tg::boundingbox &box);

  std::optional<tg::vec3d> get_pos(int x, int y);

public:
  
  void show_model(int id, bool show) override;

protected:

  TView();

protected:
  bool _camera_dirty = true;
  float _near = 0.4, _far = 2000;
};