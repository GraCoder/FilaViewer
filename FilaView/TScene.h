#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <tvec.h>

namespace fv {

class Node;
class TView;

class TScene {
protected:
  TScene();
public:
  static std::shared_ptr<TScene> create();

  ~TScene();

  void registerHandlers(std::unordered_map<uint64_t, std::function<int(const std::string_view &)>> &handlers);

  void addNode(const std::shared_ptr<Node> &node);

  int  addShape(const std::string_view &cmd);
  int  addShape(int priType, float rad);

protected:
  std::mutex _mutex;
  std::queue<std::function<void()>> _tasks;
  std::unordered_map<uint32_t, std::shared_ptr<Node>> _nodes;
};

} // namespace fv