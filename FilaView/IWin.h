#pragma once

#include "FilaViewExport.h"
#include "IView.h"

class FILAVIEW_EXPORT IWin {
public:
  static IWin*  create(IWin *win = nullptr, bool with_border = true);
  static void   destroy(IWin *win);

  virtual uint64_t exec(bool thread) = 0;

  virtual void setupGui() = 0;
  virtual IView *view(int id = 0) = 0;

  int  loadModel(const char *file, float sz = 1);
  void showModel(int, bool);
  void createOperators();

  virtual int  handleCommand(const char *ops, int len) = 0;

  void registPick(void(*fun)(unsigned int)); 

};
