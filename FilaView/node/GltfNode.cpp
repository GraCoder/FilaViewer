#include "GltfNode.h"

#include "intern/RD_Gltf.h"

GltfNode::GltfNode(const std::string &file)
  : Node()
  , _file(file)
{
}

const std::unique_ptr<RDNode> &GltfNode::get_rd(bool create)
{
  if (_rd == nullptr && create) {
    //auto ptr = std::static_pointer_cast<GltfNode>(shared_from_this());
    _rd = std::make_unique<RD_Gltf>(this);
  }

  return _rd;
}
