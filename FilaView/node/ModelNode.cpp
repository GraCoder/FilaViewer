#include "node/ModelNode.h"

#include "intern/RD_Model.h"

ModelNode::ModelNode(const std::string &file)
  : Node()
  , _file(file)
{
}

RDNode * ModelNode::get_rd(bool create)
{
  if (_rd == nullptr && create) {
    //auto ptr = std::static_pointer_cast<ModelNode>(shared_from_this());
    _rd = new RD_Model(this);
  }

  return _rd;
}
