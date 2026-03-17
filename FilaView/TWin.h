#pragma once

#include <string_view>
#include <functional>

#include "FilaViewExport.h"
#include "IWin.h"
#include "TView.h"

namespace fv {

class FILAVIEW_EXPORT TWin : public IWin
{
protected:
  TWin();
public:
  virtual ~TWin();

  enum Flag { en_Frameless = 0x1, en_SetupGui = 0x2 };
  uint32_t flags() { return _flags; }
  void setFlags(uint32_t flags) { _flags = flags; }

  void resize(int w, int h);

  void realizeContext();

public:
  uint64_t winId() override;

  TView *view(int id = 0) override;

  void exec(bool thread = false) override;

  int handleCommand(const char *ops, int len) override;

protected:
  uint32_t _flags = 0;

  uint32_t _width = 800, _height = 600;

protected:
  std::unordered_map<uint64_t, std::function<int(const std::string_view &)>> _handlers;
};

} // namespace fv