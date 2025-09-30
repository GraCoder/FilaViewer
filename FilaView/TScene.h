#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tvec.h>

namespace fv {

class Node;
class TView;

class TScene {
public:
  static std::shared_ptr<TScene> create();

  ~TScene();

  void addNode(const std::shared_ptr<Node> &node);

protected:

  TScene();

protected:

};

} // namespace fv