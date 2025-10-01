#include "IWin.h"
#include "FTScene.h"
#include "FTView.h"
#include "FTWin.h"

#include "operator/PickOperator.h"

#include "nlohmann/json.hpp"

int IWin::loadModel(const char *file, float sz)
{
  auto scene = static_cast<fv::FTWin *>(this)->view(0)->scene();
  return scene->loadModel(file, sz);
}

int IWin::exeOperator(const char *ops, int len)
{
  auto js = nlohmann::json::parse(std::string(ops, len));
  auto iter = js.find("OperType");
  if (iter == js.end())
    return -1;
  if (iter.value() == 1000) {
    auto scene = static_cast<fv::FTWin *>(this)->view(0)->scene();
    return scene->addShape(js.at("PrimType"));
  } else if (iter.value() == 1001) {
  }
  return 0;
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
