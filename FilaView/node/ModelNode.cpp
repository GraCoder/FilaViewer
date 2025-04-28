#include "node/ModelNode.h"

#include "intern/RD_Model.h"

ModelNode::ModelNode(const std::string &file)
  : Node()
  , _file(file)
{
}

RDNode * ModelNode::rdnode(bool create)
{
  if (_rdnode == nullptr && create) {
    //auto ptr = std::static_pointer_cast<ModelNode>(shared_from_this());
    _rdnode = new RD_Model(this);
  }

  return _rdnode;
}
