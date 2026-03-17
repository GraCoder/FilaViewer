#pragma once

namespace filament {
class Engine;
class Material;
} // namespace filament

class NodeF
{
public:
  virtual void build(filament::Engine *engine, filament::Material const *material) = 0;
  virtual void release(filament::Engine *engine);
};