#pragma once

#include "node/RDNode.h"

#include <vector>

namespace filament {
class Engine;
}

class RD_Geometry : public RDNode{
public:
  RD_Geometry();

  void build(filament::Engine *engine);

private:
};