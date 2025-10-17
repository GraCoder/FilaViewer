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

  void setFadeDuration(float f) { _fadeDuration = f; }

private:
  bool _resetPose = false;
  float _fadeDuration = 0.5f;

  bool _resetAnimation = true;
  double _preStamp = 0, _curStamp = 0;

  int _preAnimation = -1, _curAnimation = 0;

  filament::gltfio::AssetLoader *_loader = nullptr;
  filament::gltfio::FilamentAsset *_asset = nullptr;

  absl::InlinedVector<filament::gltfio::FilamentInstance *, 4> _instances;
};

} // namespace fv} // namespace fv