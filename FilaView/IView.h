#pragma once

#include "FilaViewExport.h"

class FILAVIEW_EXPORT IView {
public:
  virtual void show_model(int id, bool show) = 0;
};