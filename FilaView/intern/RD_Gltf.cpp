#include "RD_gltf.h"

#include <filament/TransformManager.h>
#include <ktxreader/Ktx1Reader.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>
#include <gltfio/Animator.h>
#include <utils/Path.h>

#include <math/TVecHelpers.h>

using namespace filament::gltfio;

RD_gltf::RD_gltf(GltfNode *node)
  : RDNode(node)
{
}

RD_gltf::~RD_gltf() 
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

void RD_gltf::build(filament::Engine *engine)
{
  using namespace filament;
  using namespace filament::math;
  using namespace filament::math::details;

  auto node = static_cast<GltfNode *>(_node);

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

  _loader = loader;
  _asset = asset;

  loadResources(engine, asset, node->file());

  //_scene->addEntities(asset->getLightEntities(), asset->getLightEntityCount());
  auto &tm = engine->getTransformManager();
  {
    auto root = tm.getInstance(asset->getRoot());
    auto instance = asset->getInstance();
    auto aabb = instance ? instance->getBoundingBox() : asset->getBoundingBox();
    float s = _node->init_size() / (length(aabb.extent()) * 2);
    filament::math::mat4f transform = mat4f::scaling(float3(s, s, s));
    tm.setTransform(root, transform);
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

void RD_gltf::update(double timestamp) 
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

void RD_gltf::release(filament::Engine *engine)
{
  if(_loader) {
    if (_asset) {
      _loader->destroyAsset(_asset);
      _asset = nullptr;
    }
  }
  Base::release(engine);
}
