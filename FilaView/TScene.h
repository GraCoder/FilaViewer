#pragma once

#include <map>
#include <memory>
#include <string>
#include <functional>
#include <tvec.h>

namespace fpc {
class PCDiv;
class PCTile;
class PCNode;
class PCDispatch;
} // namespace fpc

class TView;

class TScene {
public:
  static std::shared_ptr<TScene> create(TView *view);

  ~TScene();

  void add_pc(const std::shared_ptr<fpc::PCNode> &node);

public:

  void get_pos(tg::vec4d[5]);
  
public:

  void clean();

  void dispatch();

protected:
  
  TScene();

protected:

#ifdef POINT_CLOUD_SUPPORT

  std::unique_ptr<fpc::PCDispatch> _dispacher;

  std::map<std::string, std::shared_ptr<fpc::PCNode>> _pcs;

#endif
};