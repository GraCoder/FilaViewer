#pragma once

#include "FilaViewExport.h"
#include "IWin.h"
#include "TView.h"

class FILAVIEW_EXPORT TWin : public IWin {
public:
  virtual ~TWin();

  void resize(int w, int h);

  void realize_context();

public:

  uint64_t handle() override;

  void exec(bool thread = false) override;

  TView *view(int id = 0) override;

protected:

  TWin() = default;

protected:

  uint32_t _width = 800, _height = 600;
};