#pragma once

#include "node/RDNode.h"

#include <filament/Engine.h>

#include "node/GltfNode.h"

namespace filament {
namespace gltfio {
class AssetLoader;
class FilamentAsset;
class FilamentInstance;
} // namespace gltfio
} // namespace filament

class RD_Gltf : public RDNode {
public:
  RD_Gltf(GltfNode *node);
  ~RD_Gltf();

  void build(filament::Engine *engine);

  void update(double timestamp) override;

  void release() override;

  void set_fade_duration(float f) { _fade_duraion = f; }

private:

  bool  _reset_pose = false;
  float _fade_duraion = 0.5f;

  bool _reset_animation = true;
  double _pre_stamp = 0, _cur_stamp = 0;

  int _pre_animation = -1, _cur_animation = 0;

  filament::gltfio::AssetLoader *_loader = nullptr;
  filament::gltfio::FilamentAsset *_asset = nullptr;

  boost::container::small_vector<filament::gltfio::FilamentInstance *, 4> _instances;
};