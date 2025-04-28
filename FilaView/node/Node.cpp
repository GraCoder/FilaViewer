#include "node/Node.h"

#include "RDNode.h"

namespace {
static int id_ = 0;
}

Node::Node()
  : _id(++id_)
{
}

Node::~Node() 
{
  if(_rdnode) {
    delete _rdnode; 
    _rdnode = nullptr;
  }
}

void Node::translation(float t[3]) 
{
  memcpy(_translation.data(), t, sizeof(tg::vec3));
}

void Node::set_translation(const float t[3]) 
{
  _translation.set(t[0], t[1], t[2]);
}

void Node::rotation(float *f)
{
  memcpy(f, &_rotation, sizeof(tg::vec3));
}

void Node::set_rotation(const float *r) 
{
  _rotation.set(r[0], r[1], r[2]);
}

void Node::scale(float *s) 
{
  memcpy(s, &_scale, sizeof(tg::vec3));
}

void Node::set_scale(const float *s) 
{
  _scale.set(s[0], s[1], s[2]);
}

