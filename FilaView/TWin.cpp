#include "TWin.h"

#include "intern/FTWin.h"
#include "intern/FTView.h"
#include "intern/FTScene.h"

#include "TDef.h"
//#include "TApp.h"
//#include "TView.h"
//#include "TScene.h"

FT_DOWNCAST(TWin)


TWin::~TWin()
{
}

uint64_t TWin::handle() { return downcast(this)->handle(); }

void TWin::resize(int w, int h)
{
  _width = w;
  _height = h;

  downcast(this)->configure_cameras();
}

void TWin::exec(bool thread) 
{
  downcast(this)->exec(thread);
}

TView *TWin::view()
{
  return downcast(this)->view();
}


void TWin::load_model(const char *file) 
{
  auto scene = downcast(this)->view()->scene();
  scene->load_model(file);
}
