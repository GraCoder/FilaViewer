#ifndef TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H
#define TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H

namespace filament {
class Engine;
class VertexBuffer;
class IndexBuffer;
class Material;
class MaterialInstance;
class Renderable;
} // namespace filament

#include <map>
#include <unordered_map>
#include <vector>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <assimp/scene.h>
#include <filamat/MaterialBuilder.h>
#include <filament/Box.h>
#include <filament/Color.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>

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

  bool load_assert(const utils::Path &path);

  void build_assert(filament::Engine *engine, const filament::Material *def_mtl,
                    const filament::Material *transparent_mtl, bool override_mtl = false);

  utils::Entity root() { return _root_entity; }

  const std::vector<utils::Entity>& renderables() const noexcept { return _renderables; }

  const float3& min_bound() { return _min_bound; }
  const float3& max_bound() { return _max_bound; }

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
    utils::Path file;

    bool snormUV0;
    bool snormUV1;
    std::vector<uint32_t> indices;
    std::vector<half4> positions;
    std::vector<short4> tangents;
    std::vector<ushort2> texCoords0;
    std::vector<ushort2> texCoords1;
    std::vector<Mesh> meshes;
    std::vector<int> parents;
  };

  template <bool SNORMUV0S, bool SNORMUV1S>
  void process_node(Asset &asset, const aiScene *scene, size_t deep, size_t matCount,
                    const aiNode *node, int parentIndex, size_t &depth) const;

  void process_materials(Asset &asset, const aiScene *scene);

  filament::Texture *create_one_texture(uint32_t textureData);

  void process_material(const aiScene *scene, const aiMaterial *material,
                        const std::string &materialName, const std::string &dirName,
                        std::map<std::string, filament::MaterialInstance *> &outMaterials) const;

private:

  std::unique_ptr<Asset> _asset;

  filament::Engine *_engine = nullptr;

  filament::math::float3 _min_bound = filament::math::float3(1.0f);
  filament::math::float3 _max_bound = filament::math::float3(-1.0f);

  filament::VertexBuffer  *_vertex_buffer = nullptr;
  filament::IndexBuffer   *_index_buffer = nullptr;

  float mDefaultMetallic = 0.0f;
  float mDefaultRoughness = 0.4f;
  filament::sRGBColor mDefaultEmissive = filament::sRGBColor({0.0f, 0.0f, 0.0f});

  filament::Texture *_def_map = nullptr;
  filament::Texture *_def_normal_map = nullptr;

  std::map<std::string, filament::MaterialInstance *> _materials;

  utils::Entity _root_entity;
  std::vector<utils::Entity> _renderables;

  std::vector<filament::Texture *> _textures;
};

#endif // TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H
