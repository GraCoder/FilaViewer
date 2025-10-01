#pragma once

#include "FilaViewExport.h"
#include "IView.h"

class FILAVIEW_EXPORT IWin {
public:
  static IWin*  create(IWin *win = nullptr, bool with_border = true);
  static void   destroy(IWin *win);

  virtual uint64_t handle() = 0;

  virtual void exec(bool thread) = 0;

  virtual void setupGui() = 0;
  virtual IView *view(int id = 0) = 0;

  int loadModel(const char *file, float sz = 1);
  int  exeOperator(const char *ops, int len);
  void createOperators();
  void registPick(void(*fun)(unsigned int)); 

};