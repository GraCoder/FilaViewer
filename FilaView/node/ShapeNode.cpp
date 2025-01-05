#include "intern/mesh/Cube.h"
#include "intern/mesh/Sphere.h"

#include "ShapeNode.h"

ShapeNode::ShapeNode() 
{
}

CubeNode::CubeNode()
  : ShapeNode()
  , _pos(0, 0, 0)
  , _size(2, 2, 2)
{
  _rd = std::make_unique<Cube>(this);
}

SphereNode::SphereNode()
  : ShapeNode()
{
  _rd = std::make_unique<Sphere>(this);
}
