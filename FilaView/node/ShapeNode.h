#pragma once

#include "node/Node.h"
#include "tvec.h"

class RDShape;

class ShapeNode : public Node 
{
public:
  ShapeNode();
};

class CubeNode : public ShapeNode {
public:
  CubeNode();

  const tg::vec3 &pos() { return _pos; }
  const tg::vec3 &size() { return _size; }
  void set_pos(const tg::vec3 &pos) { _pos = pos; }
  void set_size(const tg::vec3 &size) { _size = size; }

private:
  tg::vec3 _pos, _size;
};

class SphereNode : public ShapeNode{
public:
  SphereNode();

  const tg::vec3 &pos() { return _pos; }
  float radius() { return _radius; }
  void set_pos(const tg::vec3 &pos) { _pos = pos; }
  void set_radius(const float &radius) { _radius = radius; }

private:
  tg::vec3 _pos;
  float _radius;
};