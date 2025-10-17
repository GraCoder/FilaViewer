#pragma once

namespace filament {
class Engine;
class VertexBuffer;
class IndexBuffer;
class Material;
class MaterialInstance;
class Renderable;
} // namespace filament

#include <map>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <filamat/MaterialBuilder.h>
#include <filament/Box.h>
#include <filament/Color.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>

struct aiNode;
struct aiScene;
struct aiMaterial;
struct TextureConfig;
struct MaterialConfig;

class MeshAssimp {
public:
  using mat4f = filament::math::mat4f;
  using half4 = filament::math::half4;
  using short4 = filament::math::short4;
  using half2 = filament::math::half2;
  using ushort2 = filament::math::ushort2;
  using float3 = filament::math::float3;

  explicit MeshAssimp();
  ~MeshAssimp();

  bool loadAssert(const utils::Path &path);

  void buildAssert(filament::Engine *engine, const filament::Material *basic_mtl, const filament::Material *default_mtl, bool override_mtl = false);

  utils::Entity root() { return _rootEntity; }

  const std::vector<utils::Entity> &renderables() const noexcept { return _renderables; }

  const float3 &minBound() { return _minBound; }
  const float3 &maxBound() { return _maxBound; }

private:
  struct Part {
    size_t offset;
    size_t count;
    std::string material;
    filament::sRGBColor baseColor;
    float opacity;
    float metallic;
    float roughness;
    float reflectance;
  };

  struct Mesh {
    size_t offset;
    size_t count;
    std::vector<Part> parts;
    filament::Box aabb;
    mat4f transform;
    mat4f accTransform;
  };

  struct Asset {
    bool snorm_uv0;
    bool snorm_uv1;
    std::string file;
    std::vector<uint32_t> indices;
    std::vector<half4> positions;
    std::vector<short4> tangents;
    std::vector<ushort2> texCoords0;
    std::vector<ushort2> texCoords1;
    std::vector<Mesh> meshes;
    std::vector<int> parents;
  };

  template <bool SNORMUV0S, bool SNORMUV1S>
  void processNode(Asset &asset, const aiScene *scene, size_t deep, size_t matCount, const aiNode *node, int parentIndex, size_t &depth) const;

  void processMaterials(const aiScene *scene);

  auto loadTextures(const aiScene *scene, const aiMaterial *material, int tex_type);

  std::unique_ptr<MaterialConfig> loadMaterial(const aiScene *scene, const aiMaterial *material);

  void buildMaterial(filament::Engine *engine);

  inline bool hasTexture(uint64_t id);

  std::string shaderFromConfig(const MaterialConfig &config);

  filament::Material *createMaterialFromConfig(filament::Engine &engine, MaterialConfig &config);

  void adjustMaterialConfig(MaterialConfig *material);

private:

  std::unique_ptr<Asset> _asset;

  filament::Engine *_engine = nullptr;

  filament::math::float3 _minBound = filament::math::float3(1.0f);
  filament::math::float3 _maxBound = filament::math::float3(-1.0f);

  filament::VertexBuffer *_vertexBuffer = nullptr;
  filament::IndexBuffer *_indexBuffer = nullptr;

  float _defMetallic = 0.0f;
  float _defRoughness = 0.4f;
  filament::sRGBColor _defEmissive = filament::sRGBColor({0.0f, 0.0f, 0.0f});

  filament::Texture *_defMap = nullptr;
  filament::Texture *_defNormalMap = nullptr;

  std::map<std::string, std::unique_ptr<MaterialConfig>> _materialConfig;
  std::map<std::string, filament::MaterialInstance *> _materials;
  std::map<uint64_t, std::shared_ptr<TextureConfig>> _textureConfig;
  std::map<uint64_t, filament::Texture *> _textures;

  utils::Entity _rootEntity;
  std::vector<utils::Entity> _renderables;
};
