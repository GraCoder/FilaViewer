#pragma once

#include "TOperator.h"
#include "TView.h"
#include "tvec.h"

class PickOperator : public TOperator
{
public:
  PickOperator(TView *view);
  ~PickOperator();

  void setcb(const std::function<void(unsigned int)> &fun) { _fun = fun; }

  bool mouse_press(const SDL_MouseButtonEvent &btn);
  bool mouse_release(const SDL_MouseButtonEvent &btn);

private:
  tg::vec2 _pos;
  TView *_view = nullptr;

  std::function<void(unsigned int)> _fun;
};