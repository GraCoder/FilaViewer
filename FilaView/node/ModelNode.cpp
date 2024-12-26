#include "node/ModelNode.h"

#include "intern/RD_Model.h"

ModelNode::ModelNode(const std::string &file)
  : Node()
  , _file(file)
{
}

const std::unique_ptr<RDNode> &ModelNode::get_rd(bool create)
{
  if (_rd == nullptr && create) {
    auto ptr = std::static_pointer_cast<ModelNode>(shared_from_this());
    _rd = std::make_unique<RD_Model>(ptr);
  }

  return _rd;
}
