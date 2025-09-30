#pragma once

#include "node/Node.h"

#include <vector>

namespace filament {
class Engine;
}

namespace fv {

class Geometry : public Node {
public:
  Geometry();

  void build(filament::Engine *engine);

private:
};

} // namespace fv