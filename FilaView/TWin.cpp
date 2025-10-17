#include "TWin.h"

#include "TView.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"
#include "intern/FTWin.h"

#include "TDef.h"

namespace fv {

FT_DOWNCAST(TWin)

TWin::~TWin() {}

uint64_t TWin::handle()
{
  return downcast(this)->handle();
}

void TWin::exec(bool thread)
{
  downcast(this)->exec(thread);
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

TView *TWin::view(int id)
{
  return downcast(this)->view(id);
}

} // namespace fv
