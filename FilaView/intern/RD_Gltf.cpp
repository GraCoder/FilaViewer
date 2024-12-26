#include "RD_Gltf.h"

#include <ktxreader/Ktx1Reader.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>
#include <gltfio/Animator.h>
#include <utils/Path.h>

using namespace filament::gltfio;

RD_Gltf::RD_Gltf(const std::shared_ptr<GltfNode> &node)
  : RDNode(node)
{
}

namespace {
void loadResources(filament::Engine *engine, filament::gltfio::FilamentAsset *asset, const utils::Path &filename)
{
  using namespace filament;
  using namespace filament::gltfio;

  // Load external textures and buffers.
  std::string const gltfPath = filename.getAbsolutePath();
  ResourceConfiguration configuration = {};
  configuration.engine = engine;
  configuration.gltfPath = gltfPath.c_str();
  configuration.normalizeSkinningWeights = true;

  gltfio::ResourceLoader resourceLoader(configuration);

  auto stbDecoder = createStbProvider(engine);
  auto ktxDecoder = createKtx2Provider(engine);
  resourceLoader.addTextureProvider("image/png", stbDecoder);
  resourceLoader.addTextureProvider("image/jpeg", stbDecoder);
  resourceLoader.addTextureProvider("image/ktx2", ktxDecoder);

  if (resourceLoader.loadResources(asset))
    return;

  asset->getInstance()->recomputeBoundingBoxes();
  asset->releaseSourceData();
};
} // namespace

void RD_Gltf::build(filament::Engine *engine)
{
  using namespace filament;

  auto node = std::static_pointer_cast<GltfNode>(_node.lock());

  auto fp = fopen(node->file().c_str(), "rb");
  if (!fp)
    return;

  fseek(fp, 0, SEEK_END);
  auto sz = ftell(fp);
  std::vector<uint8_t> data(sz, 0);
  data.resize(sz);
  fseek(fp, 0, SEEK_SET);
  fread(data.data(), 1, sz, fp);
  fclose(fp);

  auto mp = gltfio::createJitShaderProvider(engine);
  auto loader = gltfio::AssetLoader::create({engine, mp});
  auto asset = loader->createAsset(data.data(), data.size());
  if (asset == nullptr)
    return;

  loadResources(engine, asset, node->file());

  //_scene->addEntities(asset->getLightEntities(), asset->getLightEntityCount());
  auto &tm = engine->getTransformManager();
  {
    // auto root = tm.getInstance(asset->getRoot());
    // auto instance = asset->getInstance();
    // auto aabb = instance ? instance->getBoundingBox() : asset->getBoundingBox();
    // filament::math::mat4f transform;
    // tm.setTransform(root, transform);
  }

  for (int i = 0, n = asset->getRenderableEntityCount(); i < n; i++) {
    auto ent = asset->getRenderableEntities()[i];
    _entities.push_back(ent.getId());
    // auto ri = rm.getInstance(ent);
    // rm.setScreenSpaceContactShadows(ri, true);
  }

  for (int i = 0, n = asset->getAssetInstanceCount(); i < n; i++) {
    _instances.push_back(asset->getAssetInstances()[i]);
  }

  asset->releaseSourceData();
}

void RD_Gltf::update(double timestamp) 
{
  if (_reset_animation) {
    _pre_stamp = _cur_stamp;
    _cur_stamp = timestamp;
    _reset_animation = false;
  }

  const double elapsedSeconds = (timestamp - _cur_stamp) / 1000.0;

  for (auto &instance : _instances) {
    Animator &animator = *instance->getAnimator();
    const size_t animationCount = animator.getAnimationCount();
    if (animationCount > 0 && _cur_animation >= 0) {
      if (_cur_animation == animationCount) {
        for (size_t i = 0; i < animationCount; i++) {
          animator.applyAnimation(i, elapsedSeconds);
        }
      } else {
        animator.applyAnimation(_cur_animation, elapsedSeconds);
      }
      if (elapsedSeconds < _fade_duraion && _pre_animation >= 0 && _pre_animation != animationCount) {
        const double previousSeconds = timestamp - _pre_stamp;
        const float lerpFactor = elapsedSeconds / _fade_duraion;
        animator.applyCrossFade(_pre_animation, previousSeconds, lerpFactor);
      }
    }
    if (_reset_pose) {
      animator.resetBoneMatrices();
    } else {
      animator.updateBoneMatrices();
    }
  }
}
