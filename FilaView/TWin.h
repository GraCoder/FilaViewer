#pragma once

#include "FilaViewExport.h"
#include "IWin.h"
#include "TView.h"

class FILAVIEW_EXPORT TWin : public IWin {
public:
  virtual ~TWin();

  enum Flag {
    en_Frameless = 0x1,
    en_SetupGui = 0x2
  };
  uint32_t flags() { return _flags; }
  void set_flags(uint32_t flags) { _flags = flags; }

  void resize(int w, int h);

  void realize_context();

public:

  uint64_t handle() override;

  void exec(bool thread = false) override;

  TView *view(int id = 0) override;

protected:

  TWin() = default;

protected:

  uint32_t _flags = 0;

  uint32_t _width = 800, _height = 600;
};