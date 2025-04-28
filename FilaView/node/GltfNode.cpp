#include "GltfNode.h"

#include "intern/RD_gltf.h"

GltfNode::GltfNode(const std::string &file)
  : Node()
  , _file(file)
{
}

RDNode * GltfNode::rdnode(bool create)
{
  if (_rdnode == nullptr && create) {
    //auto ptr = std::static_pointer_cast<GltfNode>(shared_from_this());
    _rdnode = new RD_gltf(this);
  }

  return _rdnode;
}
