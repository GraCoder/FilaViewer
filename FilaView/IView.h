#pragma once

#include "FilaViewExport.h"

class FILAVIEW_EXPORT IView {
public:
  virtual void showEntity(int id, bool show) = 0;
};