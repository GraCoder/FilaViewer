#pragma once

#include <memory>
#include "FilaViewExport.h"
#include "tvec.h"

class RDNode;
class FILAVIEW_EXPORT Node : public std::enable_shared_from_this<Node> {
public:
  Node();
  ~Node();

  uint32_t id() { return _id; }

  float init_size() { return _init_size; }
  void set_init_size(float f) { _init_size = f; }

  const tg::vec3 &translation() { return _translation; }
  void set_translation(const tg::vec3 &t) { _translation = t; }

  void translation(float t[3]); 
  void set_translation(const float f[3]);

  const tg::vec3 &rotation() { return _rotation; }
  void set_rotation(const tg::vec3 &r) { _rotation = r; }

  void rotation(float *); 
  void set_rotation(const float *);

  const tg::vec3 &scale() { return _scale; }
  void set_scale(const tg::vec3 &s) { _scale = s; }

  void scale(float *);
  void set_scale(const float *);

public:

  RDNode *rdNode() { return _rdnode; }
  virtual RDNode *rdNode(bool create) { return nullptr; }

protected:
  uint32_t _id = 0;
  RDNode *_rdnode = nullptr;

  float _init_size = 1;

  tg::vec3 _translation;
  tg::vec3 _rotation;
  tg::vec3 _scale;
};