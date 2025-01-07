#pragma once

#include <string>
#include "node/Node.h"

class ModelNode : public Node {
  friend class RD_Model;

public:
  ModelNode(const std::string &file);

  RDNode *get_rd(bool create = false) override;

  const std::string &file() { return _file; }

protected:

private:
  std::string _file;
};