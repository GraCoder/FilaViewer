#pragma once

#include <memory>

class TView;

class TWin {
public:
  static TWin* create(TWin *);

  uint64_t handle();

  ~TWin();

  void realize();

  void resize(int w, int h);

  void exec();

  TView* view() { return _view.get(); }

protected:

  TWin();

protected:

  std::shared_ptr<TView> _view = nullptr;

  uint32_t _width = 800, _height = 600;

  bool _close = false;
};