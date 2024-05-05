#pragma once

#include <memory>

#include "FilaViewExport.h"

class TView;

class FILAVIEW_EXPORT TWin {
public:
  static TWin* create(TWin *, bool with_border = true);

  uint64_t handle();

  ~TWin();

  void resize(int w, int h);

  void exec(bool thead = false);

  TView* view() { return _view.get(); }

public:

  void load_model(const char *file);

protected:

  TWin();

protected:

  std::shared_ptr<TView> _view = nullptr;

  uint32_t _width = 800, _height = 600;
};