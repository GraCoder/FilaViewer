#pragma once

#include <string>
#include "node/Node.h"

class GltfNode : public Node {
  friend class RD_gltf;

public:
  GltfNode(const std::string &file);

  RDNode *rdnode(bool create = false) override;

  const std::string &file() { return _file; }

protected:

private:
  std::string _file;
};