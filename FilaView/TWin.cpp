#include "TWin.h"

#include "TView.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"
#include "intern/FTWin.h"

#include "TDef.h"
#include "nlohmann/json.hpp"

FT_DOWNCAST(TWin)

TWin::~TWin()
{
}

uint64_t TWin::handle() { return downcast(this)->handle(); }

void TWin::exec(bool thread) 
{
  downcast(this)->exec(thread);
}

void TWin::resize(int w, int h)
{
  _width = w;
  _height = h;

  downcast(this)->configure_cameras();
}

void TWin::realize_context() 
{
  downcast(this)->realize_context();

  downcast(this)->configure_cameras();
}

TView *TWin::view(int id)
{
  return downcast(this)->view(id);
}


int TWin::load_model(const char *file, float sz) 
{
  auto scene = downcast(this)->view(0)->scene();
  return scene->load_model(file, sz);
}

int TWin::operator_s(const char *ops, int len)
{
  auto js = nlohmann::json::parse(std::string(ops, len));
  auto iter = js.find("OperType");
  if (iter == js.end())
    return -1;
  if (iter.value() == 1000) {
    auto scene = downcast(this)->view(0)->scene();
    return scene->add_shape(js.at("PrimType"));
  }
  return 0;
}
