#include "MeshAssimp.h"

#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803

#include <cstdlib>
#include <cstring>

#include <array>
#include <filesystem>
#include <iostream>

#include <filament/Color.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <math/TVecHelpers.h>
#include <math/norm.h>
#include <math/vec3.h>

#include <backend/DriverEnums.h>
#include <utils/Log.h>

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stb_image.h>

#include <spdlog/spdlog.h>

#include "pcv_mat.h"

using namespace filament;
using namespace filamat;
using namespace filament::math;
using namespace utils;

#define _AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE "$tex.file.texCoord"
#define _AI_MATKEY_GLTF_MAPPINGNAME_BASE "$tex.mappingname"
#define _AI_MATKEY_GLTF_MAPPINGID_BASE "$tex.mappingid"
#define _AI_MATKEY_GLTF_MAPPINGFILTER_MAG_BASE "$tex.mappingfiltermag"
#define _AI_MATKEY_GLTF_MAPPINGFILTER_MIN_BASE "$tex.mappingfiltermin"
#define _AI_MATKEY_GLTF_SCALE_BASE "$tex.scale"
#define _AI_MATKEY_GLTF_STRENGTH_BASE "$tex.strength"

std::map<aiTextureType, std::string> tex_name = {
  {aiTextureType_BASE_COLOR, "baseColorMap"}, {aiTextureType_DIFFUSE, "baseColorMap"}, {aiTextureType_AMBIENT, "aoMap"},
  {aiTextureType_EMISSIVE, "emissiveMap"},    {aiTextureType_NORMALS, "normalMap"},    {aiTextureType_SHININESS, "specularColorMap"},
  {aiTextureType_SPECULAR, "glossinessMap"},
};

enum class AlphaMode : uint8_t { OPAQUE, MASKED, TRANSPARENT };

struct TextureConfig {
  aiTextureType type;
  filament::backend::SamplerMinFilter minfilter;
  filament::backend::SamplerMagFilter magfilter;

  uint32_t width = 0, height = 0;
  uint8_t *pixels = nullptr;
  uint32_t channel = 4;
  filament::backend::TextureFormat format;

  TextureConfig() {}
  ~TextureConfig()
  {
    if (pixels) {
      delete[] pixels;
    }
  }
};

struct MaterialConfig {
  bool twoside = false;
  bool vertex_color = false;

  AlphaMode alpha_mode = AlphaMode::OPAQUE;
  float mask_threshold = 0.5f;

  float4 base_color;
  uint64_t tex_base_color = 0;
  uint8_t uv_base_color = 0;

  uint64_t tex_normal = 0;
  uint8_t uv_normal = 0;

  uint64_t tex_specular = 0;
  uint8_t uv_specular = 0;

  uint64_t tex_specular_color = 0;
  uint8_t uv_specular_color = 0;

  uint64_t tex_specular_factor = 0;
  uint8_t uv_specular_factor = 0;

  uint64_t tex_metallic_rough = 0;
  uint8_t uv_metallic_rough = 0;

  uint64_t tex_emissive = 0;
  uint8_t uv_emissive = 0;

  uint64_t tex_ao = 0;
  uint8_t uv_ao = 0;

  uint64_t tex_glossiness = 0;
  uint8_t uv_glossiness = 0;

  float metallic = 0;
  uint64_t tex_metallic = 0;
  uint8_t uv_metallic = 0;

  float roughness = 0.8;
  uint64_t tex_roughness = 0;
  uint8_t uv_roughness = 0;

  uint64_t tex_height = 0;
  uint8_t uv_height = 0;

  uint8_t max_uv_index() { return std::max({uv_base_color, uv_metallic_rough, uv_emissive, uv_ao, uv_normal}); }
};

void append_boolean_to_bitmask(uint64_t &bitmask, bool b)
{
  bitmask <<= 1;
  bitmask |= uint64_t(b);
}

uint64_t hash_material_config(const MaterialConfig &config)
{
  uint64_t bitmask = 0;
  memcpy(&bitmask, &config.mask_threshold, sizeof(config.mask_threshold));
  append_boolean_to_bitmask(bitmask, config.twoside);
  append_boolean_to_bitmask(bitmask, config.vertex_color);
  append_boolean_to_bitmask(bitmask, config.alpha_mode == AlphaMode::OPAQUE);
  append_boolean_to_bitmask(bitmask, config.alpha_mode == AlphaMode::MASKED);
  append_boolean_to_bitmask(bitmask, config.alpha_mode == AlphaMode::TRANSPARENT);
  append_boolean_to_bitmask(bitmask, config.uv_base_color == 0);
  append_boolean_to_bitmask(bitmask, config.uv_metallic_rough == 0);
  append_boolean_to_bitmask(bitmask, config.uv_emissive == 0);
  append_boolean_to_bitmask(bitmask, config.uv_ao == 0);
  append_boolean_to_bitmask(bitmask, config.uv_normal == 0);
  return bitmask;
}

template <typename VECTOR, typename INDEX>
Box computeTransformedAABB(VECTOR const *vertices, INDEX const *indices, size_t count, const mat4f &transform) noexcept
{
  size_t stride = sizeof(VECTOR);
  filament::math::float3 bmin(std::numeric_limits<float>::max());
  filament::math::float3 bmax(std::numeric_limits<float>::lowest());
  for (size_t i = 0; i < count; ++i) {
    VECTOR const *p = reinterpret_cast<VECTOR const *>((char const *)vertices + indices[i] * stride);
    const filament::math::float3 v(p->x, p->y, p->z);
    float3 tv = (transform * float4(v, 1.0f)).xyz;
    bmin = min(bmin, tv);
    bmax = max(bmax, tv);
  }
  return Box().set(bmin, bmax);
}
void getMinMaxUV(const aiScene *scene, const aiNode *node, float2 &minUV, float2 &maxUV, uint32_t uvIndex)
{
  for (size_t i = 0; i < node->mNumMeshes; ++i) {
    const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    if (!mesh->HasTextureCoords(uvIndex)) {
      continue;
    }
    const auto *uv = reinterpret_cast<const float3 *>(mesh->mTextureCoords[uvIndex]);
    const size_t numVertices = mesh->mNumVertices;
    const size_t numFaces = mesh->mNumFaces;
    if (numVertices == 0 || numFaces == 0) {
      continue;
    }
    if (uv) {
      for (size_t j = 0; j < numVertices; j++) {
        minUV = min(uv[j].xy, minUV);
        maxUV = max(uv[j].xy, maxUV);
      }
    }
  }
  for (size_t i = 0; i < node->mNumChildren; ++i) {
    getMinMaxUV(scene, node->mChildren[i], minUV, maxUV, uvIndex);
  }
}

template <bool SNORMUVS> static ushort2 convertUV(float2 uv)
{
  if (SNORMUVS) {
    short2 uvshort(packSnorm16(uv));
    return bit_cast<ushort2>(uvshort);
  } else {
    half2 uvhalf(uv);
    return bit_cast<ushort2>(uvhalf);
  }
}

template <typename T> struct State {
  std::vector<T> state;
  explicit State(std::vector<T> &&state)
    : state(std::move(state))
  {
  }
  static void free(void *buffer, size_t size, void *user)
  {
    auto *const that = static_cast<State<T> *>(user);
    delete that;
  }
  size_t size() const { return state.size() * sizeof(T); }
  T const *data() const { return state.data(); }
};

struct TextureImage {
  std::string _img_path;
};

MeshAssimp::MeshAssimp() {}

MeshAssimp::~MeshAssimp()
{
  if (_engine) {
    _engine->destroy(_vertex_buffer);
    _engine->destroy(_index_buffer);
    _engine->destroy(_def_map);
    _engine->destroy(_def_normal_map);

    for (Entity renderable : _renderables) {
      _engine->destroy(renderable);
    }

    for (auto &t : _textures) {
      _engine->destroy(t.second);
    }

    for (auto &m : _materials) {
      _engine->destroy(m.second);
    }
  }

  EntityManager::get().destroy(_renderables.size(), _renderables.data());
}

namespace {
Texture *create_texture(uint32_t pixel, Engine *engine)
{
  uint32_t *tex_data = (uint32_t *)malloc(sizeof(uint32_t) * 4);
  tex_data[0] = tex_data[1] = tex_data[2] = tex_data[3] = pixel;

  auto *texture = Texture::Builder().width(uint32_t(1)).height(uint32_t(1)).levels(0xff).format(Texture::InternalFormat::RGBA8).build(*engine);

  Texture::PixelBufferDescriptor buf(tex_data, size_t(1 * 1 * 4), Texture::Format::RGBA, Texture::Type::UBYTE, (Texture::PixelBufferDescriptor::Callback)&free);

  texture->setImage(*engine, 0, std::move(buf));
  return texture;
}

TextureSampler::WrapMode map_texture_mode(aiTextureMapMode map_mode)
{
  switch (map_mode) {
  case aiTextureMapMode_Clamp:
    return TextureSampler::WrapMode::CLAMP_TO_EDGE;
  case aiTextureMapMode_Mirror:
    return TextureSampler::WrapMode::MIRRORED_REPEAT;
  default:
    return TextureSampler::WrapMode::REPEAT;
  }
}

TextureSampler::MinFilter ai_minfilter_to_filament(unsigned int aiMinFilter)
{
  switch (aiMinFilter) {
  case GL_NEAREST:
    return TextureSampler::MinFilter::NEAREST;
  case GL_LINEAR:
    return TextureSampler::MinFilter::LINEAR;
  case GL_NEAREST_MIPMAP_NEAREST:
    return TextureSampler::MinFilter::NEAREST_MIPMAP_NEAREST;
  case GL_LINEAR_MIPMAP_NEAREST:
    return TextureSampler::MinFilter::LINEAR_MIPMAP_NEAREST;
  case GL_NEAREST_MIPMAP_LINEAR:
    return TextureSampler::MinFilter::NEAREST_MIPMAP_LINEAR;
  case GL_LINEAR_MIPMAP_LINEAR:
    return TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR;
  default:
    return TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR;
  }
}

TextureSampler::MagFilter ai_magfilter_to_filament(unsigned int aiMagFilter)
{
  switch (aiMagFilter) {
  case GL_NEAREST:
    return TextureSampler::MagFilter::NEAREST;
  default:
    return TextureSampler::MagFilter::LINEAR;
  }
}

} // namespace

bool MeshAssimp::load_assert(const utils::Path &path)
{
  Assimp::Importer importer;
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
  importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
  importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);

  const aiScene *scene = importer.ReadFile(path,
                                           // normals and tangents
                                           aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals |
                                             // UV Coordinates
                                             aiProcess_GenUVCoords |
                                             // topology optimization
                                             aiProcess_FindInstances | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices |
                                             // misc optimization
                                             aiProcess_ImproveCacheLocality | aiProcess_SortByPType |
                                             // we only support triangles
                                             aiProcess_Triangulate);

  if (!scene) {
    std::cout << "No scene" << std::endl;
    return false;
  }

  if (scene && !scene->mRootNode) {
    std::cout << "No root node" << std::endl;
    return false;
  }

  auto asset = std::make_unique<Asset>();
  asset->file = path;

  const std::function<void(aiNode const *node, size_t &totalVertexCount, size_t &totalIndexCount)> countVertices =
    [scene, &countVertices](aiNode const *node, size_t &totalVertexCount, size_t &totalIndexCount) {
      for (size_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh const *mesh = scene->mMeshes[node->mMeshes[i]];
        totalVertexCount += mesh->mNumVertices;

        const aiFace *faces = mesh->mFaces;
        const size_t numFaces = mesh->mNumFaces;
        totalIndexCount += numFaces * faces[0].mNumIndices;
      }

      for (size_t i = 0; i < node->mNumChildren; i++) {
        countVertices(node->mChildren[i], totalVertexCount, totalIndexCount);
      }
    };

  size_t deep = 0;
  size_t depth = 0;
  size_t matCount = 0;

  aiNode const *node = scene->mRootNode;

  size_t totalVertexCount = 0;
  size_t totalIndexCount = 0;

  countVertices(node, totalVertexCount, totalIndexCount);

  asset->positions.reserve(asset->positions.size() + totalVertexCount);
  asset->tangents.reserve(asset->tangents.size() + totalVertexCount);
  asset->texCoords0.reserve(asset->texCoords0.size() + totalVertexCount);
  asset->texCoords1.reserve(asset->texCoords1.size() + totalVertexCount);
  asset->indices.reserve(asset->indices.size() + totalIndexCount);

  float2 minUV0 = float2(std::numeric_limits<float>::max());
  float2 maxUV0 = float2(std::numeric_limits<float>::lowest());
  getMinMaxUV(scene, node, minUV0, maxUV0, 0);
  float2 minUV1 = float2(std::numeric_limits<float>::max());
  float2 maxUV1 = float2(std::numeric_limits<float>::lowest());
  getMinMaxUV(scene, node, minUV1, maxUV1, 1);

  asset->snorm_uv0 = minUV0.x >= -1.0f && minUV0.x <= 1.0f && maxUV0.x >= -1.0f && maxUV0.x <= 1.0f && minUV0.y >= -1.0f && minUV0.y <= 1.0f &&
                     maxUV0.y >= -1.0f && maxUV0.y <= 1.0f;

  asset->snorm_uv1 = minUV1.x >= -1.0f && minUV1.x <= 1.0f && maxUV1.x >= -1.0f && maxUV1.x <= 1.0f && minUV1.y >= -1.0f && minUV1.y <= 1.0f &&
                     maxUV1.y >= -1.0f && maxUV1.y <= 1.0f;

  if (asset->snorm_uv0) {
    if (asset->snorm_uv1) {
      process_node<true, true>(*asset, scene, deep, matCount, node, -1, depth);
    } else {
      process_node<true, false>(*asset, scene, deep, matCount, node, -1, depth);
    }
  } else {
    if (asset->snorm_uv1) {
      process_node<false, true>(*asset, scene, deep, matCount, node, -1, depth);
    } else {
      process_node<false, false>(*asset, scene, deep, matCount, node, -1, depth);
    }
  }

  for (auto &mesh : asset->meshes) {
    mesh.aabb = RenderableManager::computeAABB(asset->positions.data(), asset->indices.data() + mesh.offset, mesh.count);

    Box transformedAabb = computeTransformedAABB(asset->positions.data(), asset->indices.data() + mesh.offset, mesh.count, mesh.accTransform);

    float3 aabbMin = transformedAabb.getMin();
    float3 aabbMax = transformedAabb.getMax();

    if (!isinf(aabbMin.x) && !isinf(aabbMax.x)) {
      if (_min_bound.x > _max_bound.x) {
        _min_bound.x = aabbMin.x;
        _max_bound.x = aabbMax.x;
      } else {
        _min_bound.x = fmin(_min_bound.x, aabbMin.x);
        _max_bound.x = fmax(_max_bound.x, aabbMax.x);
      }
    }

    if (!isinf(aabbMin.y) && !isinf(aabbMax.y)) {
      if (_min_bound.y > _max_bound.y) {
        _min_bound.y = aabbMin.y;
        _max_bound.y = aabbMax.y;
      } else {
        _min_bound.y = fmin(_min_bound.y, aabbMin.y);
        _max_bound.y = fmax(_max_bound.y, aabbMax.y);
      }
    }

    if (!isinf(aabbMin.z) && !isinf(aabbMax.z)) {
      if (_min_bound.z > _max_bound.z) {
        _min_bound.z = aabbMin.z;
        _max_bound.z = aabbMax.z;
      } else {
        _min_bound.z = fmin(_min_bound.z, aabbMin.z);
        _max_bound.z = fmax(_max_bound.z, aabbMax.z);
      }
    }
  }

  _asset = std::move(asset);

  process_materials(scene);

  return true;
}

template <bool SNORMUV0, bool SNORMUV1>
void MeshAssimp::process_node(Asset &asset, const aiScene *scene, size_t deep, size_t matCount, const aiNode *node, int parentIndex, size_t &depth) const
{
  mat4f const &current = transpose(*reinterpret_cast<mat4f const *>(&node->mTransformation));

  size_t totalIndices = 0;
  asset.parents.push_back(parentIndex);
  asset.meshes.push_back(Mesh{});
  asset.meshes.back().offset = asset.indices.size();
  asset.meshes.back().transform = current;

  mat4f parentTransform = parentIndex >= 0 ? asset.meshes[parentIndex].accTransform : mat4f();
  asset.meshes.back().accTransform = parentTransform * current;

  for (size_t i = 0; i < node->mNumMeshes; i++) {
    aiMesh const *mesh = scene->mMeshes[node->mMeshes[i]];

    float3 const *positions = reinterpret_cast<float3 const *>(mesh->mVertices);
    float3 const *tangents = reinterpret_cast<float3 const *>(mesh->mTangents);
    float3 const *bitangents = reinterpret_cast<float3 const *>(mesh->mBitangents);
    float3 const *normals = reinterpret_cast<float3 const *>(mesh->mNormals);
    float3 const *texCoords0 = reinterpret_cast<float3 const *>(mesh->mTextureCoords[0]);
    float3 const *texCoords1 = reinterpret_cast<float3 const *>(mesh->mTextureCoords[1]);

    const size_t numVertices = mesh->mNumVertices;

    if (numVertices > 0) {
      const aiFace *faces = mesh->mFaces;
      const size_t numFaces = mesh->mNumFaces;

      if (numFaces > 0) {
        size_t indicesOffset = asset.positions.size();

        for (size_t j = 0; j < numVertices; j++) {
          float3 normal = normals[j];
          float3 tangent;
          float3 bitangent;

          // Assimp always returns 3D tex coords but we only support 2D tex coords.
          float2 texCoord0 = texCoords0 ? texCoords0[j].xy : float2{0.0};
          float2 texCoord1 = texCoords1 ? texCoords1[j].xy : float2{0.0};
          // If the tangent and bitangent don't exist, make arbitrary ones. This only
          // occurs when the mesh is missing texture coordinates, because assimp
          // computes tangents for us. (search up for aiProcess_CalcTangentSpace)
          if (!tangents) {
            bitangent = normalize(cross(normal, float3{1.0, 0.0, 0.0}));
            tangent = normalize(cross(normal, bitangent));
          } else {
            tangent = tangents[j];
            bitangent = bitangents[j];
          }

          quatf q = filament::math::details::TMat33<float>::packTangentFrame({tangent, bitangent, normal});
          asset.tangents.push_back(packSnorm16(q.xyzw));
          asset.texCoords0.emplace_back(convertUV<SNORMUV0>(texCoord0));
          asset.texCoords1.emplace_back(convertUV<SNORMUV1>(texCoord1));

          asset.positions.emplace_back(positions[j], 1.0_h);
        }

        // Populate the index buffer. All faces are triangles at this point because we
        // asked assimp to perform triangulation.
        size_t indicesCount = numFaces * faces[0].mNumIndices;
        size_t indexBufferOffset = asset.indices.size();
        totalIndices += indicesCount;

        for (size_t j = 0; j < numFaces; ++j) {
          const aiFace &face = faces[j];
          for (size_t k = 0; k < face.mNumIndices; ++k) {
            asset.indices.push_back(uint32_t(face.mIndices[k] + indicesOffset));
          }
        }

        uint32_t materialId = mesh->mMaterialIndex;
        aiMaterial const *material = scene->mMaterials[materialId];

        aiString name;
        std::string mtlname;

        if (material->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
          mtlname = AI_DEFAULT_MATERIAL_NAME;
        } else {
          mtlname = name.C_Str();
        }

        aiColor3D color;
        sRGBColor baseColor{1.0f};
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
          baseColor = *reinterpret_cast<sRGBColor *>(&color);
        }

        float opacity;
        if (material->Get(AI_MATKEY_OPACITY, opacity) != AI_SUCCESS) {
          opacity = 1.0f;
        }
        if (opacity <= 0.0f)
          opacity = 1.0f;

        float shininess;
        if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS) {
          shininess = 0.0f;
        }

        // convert shininess to roughness
        float roughness = sqrt(2.0f / (shininess + 2.0f));

        float metallic = 0.0f;
        float reflectance = 0.5f;
        if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
          // if there's a non-grey specular color, assume a metallic surface
          if (color.r != color.g && color.r != color.b) {
            metallic = 1.0f;
            baseColor = *reinterpret_cast<sRGBColor *>(&color);
          } else {
            if (baseColor.r == 0.0f && baseColor.g == 0.0f && baseColor.b == 0.0f) {
              metallic = 1.0f;
              baseColor = *reinterpret_cast<sRGBColor *>(&color);
            } else {
              // TODO: the conversion formula is correct
              // reflectance = sqrtf(color.r / 0.16f);
            }
          }
        }

        asset.meshes.back().parts.push_back({indexBufferOffset, indicesCount, mtlname, baseColor, opacity, metallic, roughness, reflectance});
      }
    }
  }

  if (node->mNumMeshes > 0) {
    asset.meshes.back().count = totalIndices;
  }

  if (node->mNumChildren) {
    deep++;
    depth = std::max(deep, depth);
    parentIndex = static_cast<int>(asset.meshes.size()) - 1;
    for (size_t i = 0, c = node->mNumChildren; i < c; i++) {
      process_node<SNORMUV0, SNORMUV1>(asset, scene, deep, matCount, node->mChildren[i], parentIndex, depth);
    }
    deep--;
  }
}

void MeshAssimp::build_assert(filament::Engine *engine, const filament::Material *basic_mtl, const filament::Material *default_mtl, bool override_mtl)
{
  if (!_asset)
    return;

  _engine = engine;

  build_materials(engine);

  {
    VertexBuffer::Builder vertexBufferBuilder = VertexBuffer::Builder()
                                                  .vertexCount((uint32_t)_asset->positions.size())
                                                  .bufferCount(4)
                                                  .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::HALF4)
                                                  .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
                                                  .normalized(VertexAttribute::TANGENTS);

    if (_asset->snorm_uv0) {
      vertexBufferBuilder.attribute(VertexAttribute::UV0, 2, VertexBuffer::AttributeType::SHORT2).normalized(VertexAttribute::UV0);
    } else {
      vertexBufferBuilder.attribute(VertexAttribute::UV0, 2, VertexBuffer::AttributeType::HALF2);
    }

    if (_asset->snorm_uv1) {
      vertexBufferBuilder.attribute(VertexAttribute::UV1, 3, VertexBuffer::AttributeType::SHORT2).normalized(VertexAttribute::UV1);
    } else {
      vertexBufferBuilder.attribute(VertexAttribute::UV1, 3, VertexBuffer::AttributeType::HALF2);
    }

    _vertex_buffer = vertexBufferBuilder.build(*_engine);

    auto ps = new State<half4>(std::move(_asset->positions));
    auto ns = new State<short4>(std::move(_asset->tangents));
    auto t0s = new State<ushort2>(std::move(_asset->texCoords0));
    auto t1s = new State<ushort2>(std::move(_asset->texCoords1));
    auto is = new State<uint32_t>(std::move(_asset->indices));

    _vertex_buffer->setBufferAt(*_engine, 0, VertexBuffer::BufferDescriptor(ps->data(), ps->size(), State<half4>::free, ps));

    _vertex_buffer->setBufferAt(*_engine, 1, VertexBuffer::BufferDescriptor(ns->data(), ns->size(), State<short4>::free, ns));

    _vertex_buffer->setBufferAt(*_engine, 2, VertexBuffer::BufferDescriptor(t0s->data(), t0s->size(), State<ushort2>::free, t0s));

    _vertex_buffer->setBufferAt(*_engine, 3, VertexBuffer::BufferDescriptor(t1s->data(), t1s->size(), State<ushort2>::free, t1s));

    _index_buffer = IndexBuffer::Builder().indexCount(uint32_t(is->size())).build(*_engine);
    _index_buffer->setBuffer(*_engine, IndexBuffer::BufferDescriptor(is->data(), is->size(), State<uint32_t>::free, is));
  }

  std::map<std::string, filament::MaterialInstance *> &materials = _materials;

  size_t startIndex = _renderables.size();
  _renderables.resize(startIndex + _asset->meshes.size());
  EntityManager::get().create(_asset->meshes.size(), _renderables.data() + startIndex);
  EntityManager::get().create(1, &_root_entity);

  TransformManager &tcm = _engine->getTransformManager();
  tcm.create(_root_entity, TransformManager::Instance{}, mat4f());

  for (auto &mesh : _asset->meshes) {
    RenderableManager::Builder builder(mesh.parts.size());
    builder.boundingBox(mesh.aabb);
    builder.screenSpaceContactShadows(true);

    size_t partIndex = 0;
    for (auto &part : mesh.parts) {
      builder.geometry(partIndex, RenderableManager::PrimitiveType::TRIANGLES, _vertex_buffer, _index_buffer, part.offset, part.count);

      if (override_mtl) {
        builder.material(partIndex, materials[AI_DEFAULT_MATERIAL_NAME]);
      } else {
        auto iter = materials.find(part.material);

        if (iter != materials.end()) {
          builder.material(partIndex, iter->second);
        } else {
          auto iter = materials.find(AI_DEFAULT_MATERIAL_NAME);
          if (iter == materials.end()) {
            auto mtl = basic_mtl->createInstance();
            materials[AI_DEFAULT_MATERIAL_NAME] = mtl;
            builder.material(partIndex, mtl);
          } else {
            builder.material(partIndex, iter->second);
          }
        }
      }
      partIndex++;
    }

    const size_t meshIndex = &mesh - _asset->meshes.data();
    Entity entity = _renderables[startIndex + meshIndex];
    if (!mesh.parts.empty()) {
      builder.build(*_engine, entity);
    }
    auto pindex = _asset->parents[meshIndex];
    TransformManager::Instance parent((pindex < 0) ? tcm.getInstance(_root_entity) : tcm.getInstance(_renderables[pindex]));
    tcm.create(entity, parent, mesh.transform);
  }
}

void MeshAssimp::process_materials(const aiScene *scene)
{
  std::map<std::string, std::unique_ptr<MaterialConfig>> mtls;
  for (int i = 0; i < scene->mNumMaterials; i++) {
    auto mtl = scene->mMaterials[i];
    std::string name = mtl->GetName().C_Str();
    auto config = load_material(scene, mtl);
    mtls[name] = std::move(config);
  }

  _material_config = std::move(mtls);
}

namespace {
std::string find_file(const std::string &path, const std::string &filename)
{
  auto f = std::filesystem::path(path).append(filename);
  if (std::filesystem::exists(f))
    return f.string();

  f = std::filesystem::path(filename).filename();
  std::vector<std::filesystem::path> dirs;
  dirs.push_back(path);

  while (!dirs.empty()) {
    std::filesystem::directory_entry d(dirs.back());
    dirs.pop_back();
    std::filesystem::directory_iterator fi(d);
    for (auto iter : fi) {
      if (iter.is_regular_file()) {
        if (iter.path().filename() == f)
          return iter.path().string();
      } else if (iter.is_directory()) {
        dirs.push_back(iter.path());
      }
    }
  }

  return "";
}

std::shared_ptr<TextureConfig> load_texture(const aiScene *scene, const std::string &dir_name, const aiString &tex_file)
{
  auto [tex, idx] = scene->GetEmbeddedTextureAndIndex(tex_file.C_Str());
  if (!tex) {
    auto pf = find_file(dir_name, tex_file.C_Str());
    if (!std::filesystem::exists(pf))
      return nullptr;

    auto tex_config = std::make_unique<TextureConfig>();
    auto f = fopen(pf.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    tex_config->width = ftell(f);
    auto ptr = new uint8_t[tex_config->width];
    fseek(f, 0, SEEK_SET);
    fread(ptr, 1, tex_config->width, f);
    tex_config->pixels = ptr;
    return tex_config;
  }

  auto tex_config = std::make_unique<TextureConfig>();
  tex_config->pixels = (uint8_t *)tex->pcData;
  tex_config->width = tex->mWidth;
  tex_config->height = tex->mHeight;

  if (strcmp(tex->achFormatHint, "rgba888") == 0) {
    tex_config->format = backend::TextureFormat::RGB8;
    tex_config->channel = 3;
  } else if (strcmp(tex->achFormatHint, "rgba8888") == 0) {
    tex_config->format = backend::TextureFormat::RGBA8;
    tex_config->channel = 4;
  }

  return tex_config;
}
} // namespace

auto MeshAssimp::load_textures(const aiScene *scene, const aiMaterial *material, int type)
{
  auto tex_type = (aiTextureType)type;
  unsigned int count = material->GetTextureCount(tex_type), uvindex = 0;

  // if (count == 0)

  aiString tex_name;
  std::string folder = std::filesystem::u8path(_asset->file).parent_path().string();

  for (int i = 0; i < count; i++) {
    if (material->GetTexture(tex_type, i, &tex_name, nullptr, &uvindex, nullptr, nullptr) != AI_SUCCESS)
      continue;

    unsigned int min_type = 0;
    unsigned int mag_type = 0;
    material->Get("$tex.mappingfiltermin", type, i, min_type);
    material->Get("$tex.mappingfiltermag", type, i, mag_type);
    auto tex = ::load_texture(scene, folder, tex_name);
    if (!tex)
      continue;
    tex->type = tex_type;
    tex->minfilter = ai_minfilter_to_filament(min_type);
    tex->magfilter = ai_magfilter_to_filament(mag_type);

    uint64_t tex_code = std::hash<std::string>()(std::string(tex_name.C_Str()) + std::to_string(type));
    _texture_config[tex_code] = tex;

    return std::tuple(tex_code, uvindex, std::string(tex_name.C_Str()));
  }

  return std::make_tuple<uint64_t, uint32_t>(0, 0, std::string(""));
}

std::unique_ptr<MaterialConfig> MeshAssimp::load_material(const aiScene *scene, const aiMaterial *material)
{
  auto mtl = std::make_unique<MaterialConfig>();
  MaterialConfig &mtl_config = *mtl;
  aiTextureMapMode map_mode[3];

  material->Get(AI_MATKEY_TWOSIDED, mtl_config.twoside);

  aiShadingMode sm;
  material->Get<aiShadingMode>(AI_MATKEY_SHADING_MODEL, sm);

  std::string tex_name;
  {
    aiColor4D color;
    material->Get(AI_MATKEY_BASE_COLOR, color);
    mtl_config.base_color = float4(color.r, color.g, color.b, color.a);
    std::tie(mtl_config.tex_base_color, mtl_config.uv_base_color, tex_name) = load_textures(scene, material, aiTextureType_BASE_COLOR);
  }

  {
    aiColor4D color;
    if (aiReturn_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color))
      mtl_config.base_color = float4(color.r, color.g, color.b, color.a);

    auto [tex, uv, n] = load_textures(scene, material, aiTextureType_DIFFUSE);
    if (tex) {
      mtl_config.tex_base_color = tex;
      mtl_config.uv_base_color = uv;
    }
  }

  {
    material->Get(AI_MATKEY_METALLIC_FACTOR, mtl_config.metallic);
    std::tie(mtl_config.tex_metallic, mtl_config.uv_metallic, tex_name) = load_textures(scene, material, aiTextureType_METALNESS);
    if (mtl_config.tex_metallic)
      spdlog::debug("metallic texture : %s", tex_name);
  }

  {
    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, mtl_config.roughness);
    std::tie(mtl_config.tex_roughness, mtl_config.uv_roughness, tex_name) = load_textures(scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
  }

  {
    std::tie(mtl_config.tex_specular_color, mtl_config.uv_specular_color, tex_name) = load_textures(scene, material, aiTextureType_SHININESS);
    if (mtl_config.tex_specular_color)
      spdlog::debug("shininess texture(specular color) : %s", tex_name);
  }

  {
    auto [tex, uv, name] = load_textures(scene, material, aiTextureType_SPECULAR);
    if (tex) {
      mtl_config.tex_glossiness = tex;
      mtl_config.uv_glossiness = uv;
    }
    if (mtl_config.tex_glossiness)
      spdlog::debug("glossiness texture : %s", name);
  }

  {
    std::tie(mtl_config.tex_height, mtl_config.uv_height, tex_name) = load_textures(scene, material, aiTextureType_HEIGHT);
  }

  {
    std::tie(mtl_config.tex_normal, mtl_config.uv_normal, tex_name) = load_textures(scene, material, aiTextureType_NORMALS);
  }

  {
    // mtl_config.tex_ao = load_texture(scene, material, folder, aiTextureType_AMBIENT);
  }

  // for (int i = 0; i < 32; i++) {
  //   load_textures(scene, material, i);
  // }

  return mtl;
}

void MeshAssimp::build_materials(filament::Engine *engine)
{
  auto build_texture = [this, engine](uint64_t tex_id) -> filament::Texture * {
    filament::Texture *texture = nullptr;

    auto iter = _textures.find(tex_id);
    if (iter != _textures.end())
      return iter->second;

    auto &tc = _texture_config[tex_id];
    filament::Texture::Builder builder;
    Texture::Format fmt = Texture::Format::RGB;
    if (tc->height == 0) {
      int width, height, channel;
      auto data = stbi_load_from_memory((unsigned char *)tc->pixels, tc->width, &width, &height, &channel, 0);
      if (data == nullptr)
        return nullptr;
      builder.width(width).height(height);
      tc->channel = channel;
      if (channel == 4) {
        fmt = Texture::Format::RGBA;
        builder.format(filament::Texture::InternalFormat::RGBA8);
      } else if (channel == 3) {
        fmt = Texture::Format::RGB;
        builder.format(filament::Texture::InternalFormat::RGB8);
      } else if (channel == 1) {
        fmt = Texture::Format::R;
        builder.format(filament::Texture::InternalFormat::R8);
      }
      texture = builder.build(*engine);
      Texture::PixelBufferDescriptor buf(data, width * height * channel, fmt, Texture::Type::UBYTE);
      texture->setImage(*engine, 0, std::move(buf));
    } else {
      builder.width(tc->width).height(tc->height);
      builder.format(tc->format);
      texture = builder.build(*engine);
      switch (tc->format) {
      case backend::TextureFormat::RGBA8:
        fmt = Texture::Format::RGBA;
        break;
      case backend::TextureFormat::RGB8:
        fmt = Texture::Format::RGB;
        break;
      }
      Texture::PixelBufferDescriptor buf(tc->pixels, size_t(tc->width * tc->height * tc->channel), fmt, Texture::Type::UBYTE,
                                         (Texture::PixelBufferDescriptor::Callback)&free);
      texture->setImage(*engine, 0, std::move(buf));
    }
    texture->generateMipmaps(*engine);
    _textures[tex_id] = texture;
    return texture;
  };

  auto fun2 = [this, engine](MaterialInstance *mtl, uint64_t tex_id) {
    {
      if (tex_id == 0)
        return;
      auto iter = _textures.find(tex_id);
      if (iter == _textures.end())
        return;
    }

    auto &tc = _texture_config[tex_id];
    TextureSampler sampler(tc->minfilter, tc->magfilter);
    auto iter = tex_name.find(tc->type);
    if (iter == tex_name.end())
      return;
    auto &name = iter->second;
    mtl->setParameter(name.c_str(), _textures[tex_id], sampler);
  };

  for (auto &iter : _texture_config)
    build_texture(iter.first);

  for (auto &iter : _material_config) {
    auto &c = iter.second;

    adjust_material_config(iter.second.get());

    auto material = create_material_from_config(*_engine, *c);
    auto mtl = material->createInstance(iter.first.c_str());
    _materials[iter.first] = mtl;

    mtl->setParameter("baseColor", RgbaType::LINEAR, c->base_color);
    mtl->setParameter("metallicFactor", c->metallic);
    mtl->setParameter("roughnessFactor", c->roughness);

    fun2(mtl, c->tex_base_color);
    fun2(mtl, c->tex_ao);
    fun2(mtl, c->tex_emissive);
    fun2(mtl, c->tex_normal);
    fun2(mtl, c->tex_specular);
    fun2(mtl, c->tex_specular_factor);
    fun2(mtl, c->tex_specular_color);
    fun2(mtl, c->tex_glossiness);
  }
}

bool MeshAssimp::has_texture(uint64_t id)
{
  return id && _textures.find(id) != _textures.end();
}

std::string MeshAssimp::shader_from_config(const MaterialConfig &config)
{
  std::string shader = R"SHADER(
        void material(inout MaterialInputs material) {
    )SHADER";

  if (has_texture(config.tex_normal)) {
    shader += "float2 uv_normal = getUV" + std::to_string(config.uv_normal) + "();\n";
    shader += R"SHADER(
      vec3 nrm_dis = texture(materialParams_normalMap, uv_normal).xyz * 2.0 - 1.0;
      nrm_dis.y = -nrm_dis.y;
      material.normal += nrm_dis;
    )SHADER";
  }

  shader += R"SHADER(
        prepareMaterial(material);
        material.baseColor = materialParams.baseColor; 
    )SHADER";

  if (has_texture(config.tex_base_color)) {
    shader += "float2 uv_base_color = getUV" + std::to_string(config.uv_base_color) + "();\n";
    shader += R"SHADER(
      vec4 baseColor = texture(materialParams_baseColorMap, uv_base_color);
      material.baseColor = baseColor;
    )SHADER";
  }

  if (has_texture(config.tex_specular_color)) {
    shader += "float2 uv_specular_color = getUV" + std::to_string(config.uv_specular_color) + "();\n";
    shader += R"SHADER(
      vec3 specularColor = texture(materialParams_specularColorMap, uv_specular_color).rgb;
      material.specularColor = specularColor; 
    )SHADER";
  }

  if (has_texture(config.tex_glossiness)) {
    shader += "float2 uv_glossiness = getUV" + std::to_string(config.uv_glossiness) + "();\n";
    shader += R"(
      float glossiness = texture(materialParams_glossinessMap, uv_glossiness).r;
      material.glossiness = glossiness;
    )";
  }

  // shader += "float2 uv_ao = getUV" + std::to_string(config.uv_ao) + "();\n";
  // shader += "float2 uv_emissive = getUV" + std::to_string(config.uv_emissive) + "();\n";

  // shader += "float2 uv_metallic_rough = getUV" + std::to_string(config.uv_metallic_rough) + "();\n";
  shader += R"SHADER(
        //material.metallic = materialParams.metallicFactor;
        //material.roughness = materialParams.roughnessFactor;
        //material.roughness = materialParams.roughnessFactor * metallicRoughness.g;
        //material.metallic = materialParams.metallicFactor * metallicRoughness.b;
        //vec4 metallicRoughness = texture(materialParams_metallicRoughnessMap, uv_metallic_rough);
        //material.ambientOcclusion = texture(materialParams_aoMap, uv_ao).r;
        //material.emissive.rgb = texture(materialParams_emissiveMap, uv_emissive).rgb;
        //material.emissive.rgb *= materialParams.emissiveFactor.rgb;
        //material.emissive.a = 0.0;
    )SHADER";

  // if (config.tex_shiness) {
  //   shader += "float2 uv_roughness = getUV" + std::to_string(config.uv_roughness) + "();\n";
  //   shader += "vec3 rclr = texture(materialParams_roughnessMap, uv_roughness).rgb;\n";
  //   shader += "material.roughness = 1.0 - (0.213 * rclr.r + 0.715 * rclr.g + 0.072 * rclr.b);\n";
  // }

  shader += "}\n";

  return shader;
}

filament::Material *MeshAssimp::create_material_from_config(Engine &engine, MaterialConfig &config)
{
  std::string shader = shader_from_config(config);
  MaterialBuilder::init();
  MaterialBuilder builder;
  builder.name("material")
    .targetApi(MaterialBuilder::TargetApi::VULKAN)
    .material(shader.c_str())
    .doubleSided(config.twoside)
    .require(VertexAttribute::UV0)
    .parameter("baseColor", MaterialBuilder::UniformType::FLOAT4)
    .parameter("metallicFactor", MaterialBuilder::UniformType::FLOAT)
    .parameter("roughnessFactor", MaterialBuilder::UniformType::FLOAT);

  if (config.tex_base_color)
    builder.parameter("baseColorMap", MaterialBuilder::SamplerType::SAMPLER_2D);

  if (config.tex_normal)
    builder.parameter("normalMap", MaterialBuilder::SamplerType::SAMPLER_2D);

  //.parameter("metallicRoughnessMap", MaterialBuilder::SamplerType::SAMPLER_2D)
  //.parameter("normalScale", MaterialBuilder::UniformType::FLOAT)
  //.parameter("aoStrength", MaterialBuilder::UniformType::FLOAT)
  //.parameter("emissiveFactor", MaterialBuilder::UniformType::FLOAT3)

  if (has_texture(config.tex_ao))
    builder.parameter("aoMap", MaterialBuilder::SamplerType::SAMPLER_2D);
  if (has_texture(config.tex_emissive))
    builder.parameter("emissiveMap", MaterialBuilder::SamplerType::SAMPLER_2D);

  if (has_texture(config.tex_specular_color)) {
    builder.parameter("specularColorMap", MaterialBuilder::SamplerType::SAMPLER_2D);
  }
  if (has_texture(config.tex_glossiness)) {
    builder.parameter("glossinessMap", MaterialBuilder::SamplerType::SAMPLER_2D);
  }

  if (config.max_uv_index() > 0) {
    builder.require(VertexAttribute::UV1);
  }

  switch (config.alpha_mode) {
  case AlphaMode::MASKED:
    builder.blending(MaterialBuilder::BlendingMode::MASKED);
    builder.maskThreshold(config.mask_threshold);
    break;
  case AlphaMode::TRANSPARENT:
    builder.blending(MaterialBuilder::BlendingMode::TRANSPARENT);
    break;
  default:
    builder.blending(MaterialBuilder::BlendingMode::OPAQUE);
  }

  if (config.tex_specular_color || config.tex_glossiness)
    builder.shading(Shading::SPECULAR_GLOSSINESS);
  else
    builder.shading(Shading::LIT);

  using namespace filament;
  // builder.shading(Shading::UNLIT);
  // builder.variantFilter((filament::UserVariantFilterMask)filament::UserVariantFilterBit::ALL);
  builder.variantFilter((UserVariantFilterMask)(UserVariantFilterBit::FOG | UserVariantFilterBit::VSM | UserVariantFilterBit::DYNAMIC_LIGHTING |
                                                UserVariantFilterBit::STE | UserVariantFilterBit::SHADOW_RECEIVER));

  Package pkg = builder.build(engine.getJobSystem());
  return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
}

void MeshAssimp::adjust_material_config(MaterialConfig *material)
{
  if (auto id = material->tex_height) {
    if (_texture_config[id]->channel == 3) {
      material->tex_normal = material->tex_height;
      material->uv_normal = material->uv_height;
      material->tex_height = 0;
      material->uv_height = 0;
      _texture_config[id]->type = aiTextureType_NORMALS;
    } else if (_texture_config[id]->channel != 1) {
      material->tex_height = 0;
    }
  }
}
