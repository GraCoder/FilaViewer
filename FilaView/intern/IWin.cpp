#include "IWin.h"
#include "FTScene.h"
#include "FTView.h"
#include "FTWin.h"

#include "operator/PickOperator.h"

int IWin::loadModel(const char *file, float sz)
{
  auto scene = static_cast<fv::FTWin *>(this)->view(0)->scene();
  return scene->loadModel(file, sz);
}

void IWin::showModel(int id, bool b) 
{
  auto scene = static_cast<fv::FTWin *>(this)->view(0)->scene();
  return scene->showEntity(id, b);
}

void IWin::createOperators()
{
  using fv::FTWin;
  auto view = static_cast<FTWin *>(this)->view(0);
  auto &ops = static_cast<FTWin *>(this)->operators();
  ops.emplace_back(std::make_shared<fv::PickOperator>(view));
}

void IWin::registPick(void (*fun)(unsigned int))
{
  auto ops = static_cast<fv::FTWin *>(this)->operators();
  for (auto &op : ops) {
    if (auto pick = dynamic_cast<fv::PickOperator *>(op.get()))
      pick->setcb(std::bind(fun, std::placeholders::_1));
  }
}
