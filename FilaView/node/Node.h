#pragma once

#include <memory>
#include <absl/container/inlined_vector.h>

namespace filament {
class Engine;
class Material;
} // namespace filament

namespace fv {

class Node {
  constexpr static int entity_size = 4;

public:
  Node();

  uint32_t id() const { return _id; }

  using EntityVector = absl::InlinedVector<uint32_t, entity_size>;
  const EntityVector &entities() { return _entities; };

  virtual void update(double timestamp) {};

  virtual void build(filament::Engine *engine, filament::Material const *material);
  virtual void release(filament::Engine *engine);

protected:

  uint32_t _id = 0;

  EntityVector _entities;
};

} // namespace fv