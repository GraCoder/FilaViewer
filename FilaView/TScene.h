#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tvec.h>

namespace fpc {
class PCDiv;
class PCTile;
class PCNode;
class PCDispatch;
} // namespace fpc

class Node;
class TView;

class TScene {
public:
  static std::shared_ptr<TScene> create();

  ~TScene();

  void add_node(const std::shared_ptr<Node> &node);

  void add_pointcloud(const std::shared_ptr<fpc::PCNode> &node);

public:

  void get_pos(tg::vec4d[5]);

public:

  void release();

  void dispatch();

protected:

  TScene();

protected:

#ifdef POINT_CLOUD_SUPPORT

  std::unique_ptr<fpc::PCDispatch> _dispacher;

  std::map<std::string, std::shared_ptr<fpc::PCNode>> _pcs;

#endif
};