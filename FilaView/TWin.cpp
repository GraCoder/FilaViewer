#include "TWin.h"

#include "TView.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"
#include "intern/FTWin.h"

#include "TDef.h"

#include "nlohmann/json.hpp"

namespace fv {

FT_DOWNCAST(TWin)

TWin::TWin() {}

TWin::~TWin() {}

uint64_t TWin::winId()
{
  return downcast(this)->winId();
}

TView *TWin::view(int id)
{
  return downcast(this)->view(id);
}

void TWin::exec(bool thread)
{
  downcast(this)->exec(thread);
}

int TWin::handleCommand(const char *ops, int len)
{
  std::string_view op(ops, len);
  auto js = nlohmann::json::parse(op);
  auto iter = js.find("Name");
  if (iter == js.end())
    return -1;

  auto cmdId = std::hash<std::string>{}(iter.value());
  auto citer = _handlers.find(cmdId);
  if (citer == _handlers.end())
    return -1;

  return citer->second(ops);

  if (iter.value() == "AddCube") {
  }
  auto scene = static_cast<fv::FTWin *>(this)->view(0)->scene();
  return 0;
}

void TWin::resize(int w, int h)
{
  _width = w;
  _height = h;

  downcast(this)->configCamera();
}

void TWin::realizeContext()
{
  downcast(this)->realizeContext();

  downcast(this)->configCamera();
}

} // namespace fv
