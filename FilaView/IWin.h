#pragma once

#include "FilaViewExport.h"
#include "IView.h"

class FILAVIEW_EXPORT IWin {
public:
  static IWin*  create(IWin *win = nullptr, bool with_border = true);
  static void   destroy(IWin *win);

  virtual uint64_t handle() = 0;

  virtual void exec(bool thread) = 0;

  virtual IView *view(int id = 0) = 0;

  virtual int load_model(const char *file, float sz = 0) = 0;

  virtual int operator_s(const char *ops, int len) = 0;
};