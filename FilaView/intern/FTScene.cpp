#include "FTScene.h"

#include <fstream>
#include <filesystem>
#include <memory>

#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/LightManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/IndirectLight.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <ibl/CubemapUtils.h>
#include <ktxreader/Ktx1Reader.h>

#include "FTView.h"

#include "pcv_mat.h"
#ifdef HAS_ENVIRONMENT
#include "ibl.h"
#include "skybox.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

#include "imgui/imgui.h"

#include "node/mesh/Cube.h"
#include "node/mesh/Sphere.h"
#include "node/GltfNode.h"
#include "node/MeshNode.h"
#include "node/Geometry.h"
#include "node/ShapeNode.h"

using namespace filament;
using namespace filament::math;

namespace {

mat4f fit_to_unit_cube(const Aabb &bounds, float zoffset)
{
  float3 minpt = bounds.min;
  float3 maxpt = bounds.max;
  float maxExtent;
  maxExtent = std::max(maxpt.x - minpt.x, maxpt.y - minpt.y);
  maxExtent = std::max(maxExtent, maxpt.z - minpt.z);
  float scaleFactor = 2.0f / maxExtent;
  float3 center = (minpt + maxpt) / 2.0f;
  center.z += zoffset / scaleFactor;
  return mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
}

std::vector<uint8_t> readfile(const std::string &path)
{
  std::ifstream file(path, std::ios::binary);
  return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), {});
}

constexpr float3 irr_sh[] = {
(0.716741859912872, 0.667163610458374, 0.617367982864380),                              // L00, irradiance, pre-scaled base
(0.416345566511154, 0.402111917734146, 0.396503597497940),                              // L1-1, irradiance, pre-scaled base
(0.913356125354767, 0.771790504455566, 0.604631900787354),                              // L10, irradiance, pre-scaled base
(-0.672907054424286, -0.567880451679230, -0.443705767393112),                           // L11, irradiance, pre-scaled base
(-0.350750058889389, -0.298378944396973, -0.236268237233162),                           // L2-2, irradiance, pre-scaled base
(0.482089698314667, 0.409613221883774, 0.325085431337357),                              // L2-1, irradiance, pre-scaled base
(0.114844463765621, 0.095943816006184, 0.073779888451099),                              // L20, irradiance, pre-scaled base
(-0.794750630855560, -0.675792217254639, -0.537204325199127),                           // L21, irradiance, pre-scaled base
(0.099179431796074, 0.087250791490078, 0.070927888154984)                               // L22, irradiance, pre-scaled base
}; // namespace

} // namespace

namespace fv {

FTScene::FTScene()
  : TScene()
{
}

FTScene::~FTScene()
{
  for (auto &iter : _nodes) {
    iter.second->release(_engine);
  }

  _engine->destroy(_default_material);
  _engine->destroy(_basic_material);
  _engine->destroy(_scene);
  _engine->destroy(_skybox_tex);
  _engine->destroy(_skybox);
  _engine->destroy(_ibl_tex);
  _engine->destroy(_ibl);

  _engine->destroy(utils::Entity::import(_sunLight));
}

void FTScene::show_box(const tg::boundingbox &box)
{
#if 0
  if (!_cube)
    return;

  auto &tcm = _engine.getTransformManager();
  auto ti = tcm.getInstance(_cube->wire_entity());
  tg::vec3 ct = box.center(), sz = box.max() - box.min();
  auto fm = math::mat4f::translation<float>(math::float3(ct.x(), ct.y(), ct.z())) *
            math::mat4f::scaling<float>(math::float3(sz.x(), sz.y(), sz.z()));
  tcm.setTransform(ti, fm);
#endif
}

void FTScene::set_environment(const std::string_view &prefix, bool filter)
{
  using namespace filament;

  auto light = utils::EntityManager::get().create();
  LightManager::Builder(LightManager::Type::SUN)
    .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
    .intensity(100000)
    .direction({1.0, -1.0, 0})
    .sunAngularRadius(1.9f)
    .castShadows(false)
    .sunHaloSize(10.0f)
    .sunHaloFalloff(80.f)
    .build(*_engine, light);
  _scene->addEntity(light);
  _sunLight = light.getId();

  auto create_ktx = [](const std::string &path) {
    using namespace std;
    ifstream file(path, ios::binary);
    vector<uint8_t> contents((istreambuf_iterator<char>(file)), {});
    return new image::Ktx1Bundle(contents.data(), contents.size());
  };

  image::Ktx1Bundle *ibl_ktx = nullptr;
  image::Ktx1Bundle *skybox_ktx = nullptr;

  std::string ibl_path = std::string(prefix) + "_ibl.ktx";
  std::string skybox_path = std::string(prefix) + "_skybox.ktx";
  if (std::filesystem::exists(ibl_path) && std::filesystem::exists(skybox_path)) {
    ibl_ktx = create_ktx(ibl_path);
    skybox_ktx = create_ktx(skybox_path);
  } else {
#ifdef HAS_ENVIRONMENT
    ibl_ktx = new image::Ktx1Bundle(::ibl, ibl_length);
    skybox_ktx = new image::Ktx1Bundle(skybox, skybox_length);
#endif
  }

  Skybox::Builder skyBuilder;
  skyBuilder.showSun(true);

  if (ibl_ktx && skybox_ktx) {
    _ibl_tex = ktxreader::Ktx1Reader::createTexture(_engine, ibl_ktx, true);
    _ibl = IndirectLight::Builder().irradiance(3, irr_sh).reflections(_ibl_tex).intensity(30000).build(*_engine);
    _scene->setIndirectLight(_ibl);

    _skybox_tex = ktxreader::Ktx1Reader::createTexture(_engine, skybox_ktx, true);
    //_skybox_tex->generateMipmaps(*_engine);
    //_skybox->setLayerMask(0x7, 0x4);
    skyBuilder.environment(_skybox_tex);
  }
  _skybox = skyBuilder.build(*_engine);
  _scene->setSkybox(_skybox);
}

int FTScene::loadModel(const std::string &file, float size)
{
  if (file.ends_with("gltf") || file.ends_with("glb")) {
    auto node = std::make_shared<GltfNode>();
    std::unique_lock<std::mutex> lock(_mutex);
    _tasks.push(std::bind(
      [this, file](const std::shared_ptr<GltfNode> &node) {
        node->build(file, _engine);
        addNode(node);
      },
      node));
    return node->id();
  }

  auto node = std::make_shared<MeshNode>();
  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push(std::bind([this, file](const std::shared_ptr<MeshNode> &node) {
    node->build(file, _engine, _basic_material, _default_material);
    addNode(node);
    },
    node));

  return node->id();
}

void FTScene::showModel(int id, bool show)
{
  auto iter = _nodes.find(id);
  if (iter == _nodes.end())
    return;
  auto &rds = iter->second->entities();
  auto &rm = _engine->getRenderableManager();
  for (auto &e : rds) {
    auto instance = rm.getInstance(utils::Entity::import(e));
    auto mask = rm.getLayerMask(instance);
    rm.setLayerMask(instance, 0x255, show ? 1 : 0);
  }
}

int FTScene::addShape(int pri)
{
  std::shared_ptr<ShapeNode> node;
  if (pri == 0) {
    node = std::make_shared<ShapeNode>(std::make_unique<Cube>(math::float3{0, 0, 0}, math::float3{0.5, 0.5, 0.5}));
  } else if (pri == 1) {
    node = std::make_shared<ShapeNode>(std::make_unique<Sphere>(math::float3{0, 0, 0}, 0.5));
  }

  if (!node)
    return -1;

  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push(std::bind([this, node]() {
    node->build(_engine, _default_material);
    _add_node(node);
  }));
  return node->id();
}

std::shared_ptr<Node> FTScene::find_node(uint32_t rent)
{
  for (auto &iter : _nodes) {
    auto rds = iter.second->entities();
    for (auto &r : rds) {
      if (r == rent)
        return iter.second;
    }
  }
  return nullptr;
}

void FTScene::_add_node(const std::shared_ptr<Node> &node)
{
  _nodes.insert_or_assign(node->id(), node);

  auto &ents = node->entities();

  for (auto ent : ents) {
    _scene->addEntity(utils::Entity::import(ent));
  }
}

void FTScene::initialize(filament::Engine *engine)
{
  if (_initialized)
    return;

  _initialized = true;

  _engine = engine;
  _scene = _engine->createScene();

  {
    auto basic_mtl = filament::Material::Builder().package(PCV_MAT_BASIC_DATA, PCV_MAT_BASIC_SIZE).build(*_engine);
    basic_mtl->setDefaultParameter("baseColor", RgbType::LINEAR, float3{0.8});
    _basic_material = basic_mtl;

    auto def_mtl = filament::Material::Builder().package(PCV_MAT_DEFAULT_DATA, PCV_MAT_DEFAULT_SIZE).build(*_engine);
    def_mtl->setDefaultParameter("baseColor", RgbType::LINEAR, float3{0.8});
    def_mtl->setDefaultParameter("metallic", 0.0f);
    def_mtl->setDefaultParameter("roughness", 0.4f);
    def_mtl->setDefaultParameter("reflectance", 0.5f);
    _default_material = def_mtl;

    auto legacy_mtl = filament::Material::Builder().package(PCV_MAT_LEGACY_DATA, PCV_MAT_LEGACY_SIZE).build(*_engine);
    _legacy_material = legacy_mtl;
  }

  set_environment();
  // set_environment("C:\\Users\\t\\dev\\0\\filament\\samples\\assets\\ibl\\lightroom_14b\\lightroom_14b");
}
void FTScene::process(double timestamp)
{
  for (auto &iter : _nodes) {
    iter.second->update(timestamp);
  }

  if (_tasks.empty())
    return;

  std::unique_lock<std::mutex> lock{_mutex, std::try_to_lock};
  if (!lock.owns_lock())
    return;

  int count = 4;
  while (_tasks.size() > 0 && count-- > 0) {
    _tasks.front()();
    _tasks.pop();
  }
}

} // namespace fv
