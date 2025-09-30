#pragma once

#include <filament/Engine.h>
#include "node/Node.h"

namespace filament {
namespace gltfio {
class AssetLoader;
class FilamentAsset;
class FilamentInstance;
} // namespace gltfio
} // namespace filament


namespace fv {

class GltfNode : public Node {
  using Base = Node;

public:
  GltfNode();
  ~GltfNode();

  void build(const std::string &file, filament::Engine *engine);

  void update(double timestamp) override;

  void release(filament::Engine *) override;

  void setFadeDuration(float f) { _fade_duraion = f; }

private:

  bool _reset_pose = false;
  float _fade_duraion = 0.5f;

  bool _reset_animation = true;
  double _pre_stamp = 0, _cur_stamp = 0;

  int _pre_animation = -1, _cur_animation = 0;

  filament::gltfio::AssetLoader *_loader = nullptr;
  filament::gltfio::FilamentAsset *_asset = nullptr;

  absl::InlinedVector<filament::gltfio::FilamentInstance *, 4> _instances;
};

} // namespace fv