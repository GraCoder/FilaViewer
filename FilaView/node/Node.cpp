#include "node/Node.h"

#include "RDNode.h"

namespace {
static int id_ = 0;
}

Node::Node()
  : _id(++id_)
{
}

Node::~Node() {}