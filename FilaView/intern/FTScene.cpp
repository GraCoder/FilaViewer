#include "FTScene.h"

#include <fstream>

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
//#include "res/ibl.h"
//#include "res/skybox.h"

#include "mesh/Cube.h"
#include "mesh/Sphere.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

#include "imgui/imgui.h"

#ifdef POINT_CLOUD_SUPPORT
#include "PCDispatch.h"
#include "PCNode.h"
#endif

#include "intern/mesh/RDShape.h"
#include "intern/RD_gltf.h"
#include "intern/RD_Model.h"
#include "intern/RD_Geometry.h"

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
  (0.709325432777405, 0.658747255802155, 0.610359013080597),    // L00, irradiance, pre-scaled base
  (0.417028665542603, 0.403681904077530, 0.398497104644775),    // L1-1, irradiance, pre-scaled base
  (0.894471466541290, 0.753924071788788, 0.592269182205200),    // L10, irradiance, pre-scaled base
  (-0.665116548538208, -0.563856363296509, -0.437892466783524), // L11, irradiance, pre-scaled base
  (-0.343768268823624, -0.292864233255386, -0.232038006186485), // L2-2, irradiance, pre-scaled base
  (0.473459452390671, 0.403695404529572, 0.323223799467087),    // L2-1, irradiance, pre-scaled base
  (0.110022500157356, 0.091711401939392, 0.070404306054115),    // L20, irradiance, pre-scaled base
  (-0.770592808723450, -0.657294690608978, -0.521079838275909), // L21, irradiance, pre-scaled base
  (0.096141062676907, 0.084442004561424, 0.068696990609169)     // L22, irradiance, pre-scaled base
};

} // namespace

FTScene::FTScene()
  : TScene()
{
}

FTScene::~FTScene()
{
  for (auto &iter : _nodes) {
    auto rd = iter.second->rdNode();
    if (rd == nullptr)
      continue;
    rd->release(_engine);
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

void FTScene::set_environment(const std::string &img_path, bool filter)
{
  using namespace filament;

  auto light = utils::EntityManager::get().create();
  LightManager::Builder(LightManager::Type::SUN)
    .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
    .intensity(110000)
    .direction({-1.0, -1.0, 0})
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

  //image::Ktx1Bundle *skybox_ktx = nullptr;
  //if(img_path.empty())
  //  skybox_ktx = new image::Ktx1Bundle(skybox, skybox_length);
  //else {
  //  auto sky_path = img_path + "_skybox.ktx";
  //  skybox_ktx = create_ktx(sky_path);
  //}
  //_skybox_tex = ktxreader::Ktx1Reader::createTexture(_engine, skybox_ktx, true);
  //_skybox_tex->generateMipmaps(*_engine);
  _skybox = Skybox::Builder()
    .environment(_skybox_tex)
    .showSun(true)
    .build(*_engine);
  _scene->setSkybox(_skybox);

  //image::Ktx1Bundle *ibl_ktx = nullptr;
  //if(img_path.empty())
  //  ibl_ktx = new image::Ktx1Bundle(::ibl, ibl_length);
  //else
  //  ibl_ktx = create_ktx(img_path + "_ibl.ktx");
  //_ibl_tex = ktxreader::Ktx1Reader::createTexture(_engine, ibl_ktx, true);
  _ibl = IndirectLight::Builder()
    .irradiance(3, irr_sh)
    .reflections(_ibl_tex)
    .intensity(30000)
    .build(*_engine);
  _scene->setIndirectLight(_ibl);
}

int FTScene::load_model(const std::string &file, float size)
{
  if (file.ends_with("gltf") || file.ends_with("glb")) {
    auto node = std::make_shared<GltfNode>(file);
    node->set_init_size(size);
    std::unique_lock<std::mutex> lock(_mutex);
    _tasks.push(std::bind(
      [this](const std::shared_ptr<GltfNode> &node) {
        auto rd = static_cast<RD_gltf *>(node->rdNode(true));
        rd->build(_engine);
        add_node(node);
      },
      node));
    return node->id();
  }

  auto node = std::make_shared<ModelNode>(file);
  node->set_init_size(size);
  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push([this, node]() { 
    auto rd = static_cast<RD_Model *>(node->rdNode(true));
    rd->build(_engine, _basic_material, _default_material);
    add_node(node);
  });

  return node->id();
}

void FTScene::show_model(int id, bool show) 
{
  auto iter = _nodes.find(id);
  if (iter == _nodes.end())
    return;
  auto &rds = iter->second->rdNode()->renderables();
  auto &rm = _engine->getRenderableManager();
  for(auto &e : rds) {
    auto instance = rm.getInstance(utils::Entity::import(e));
    auto mask = rm.getLayerMask(instance);
    rm.setLayerMask(instance, 0x255, show ? 1 : 0);
  }
}

int FTScene::add_shape(int pri)
{
  std::shared_ptr<ShapeNode> node;
  if (pri == 0) {
    node = std::make_shared<CubeNode>();
  } else if(pri == 1) {
    node = std::make_shared<SphereNode>();
  }

  if (!node)
    return -1;

  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push(std::bind([this, node]() {
    auto rd = static_cast<RDShape *>(node->rdNode());
    rd->build(_engine, _default_material);
    _add_node(node);
  }));
  return node->id();
}

std::shared_ptr<Node> FTScene::find_node(uint32_t rent)
{
  for (auto &iter : _nodes) {
    auto rd = iter.second->rdNode();
    if (!rd)
      continue;
    auto rds = rd->renderables();
    for (auto &r : rds) {
      if (r == rent)
        return iter.second;
    }
  }
  return nullptr;
}

void FTScene::_add_node(const std::shared_ptr<Node> &node) 
{
  auto rd = node->rdNode();
  if (!rd) return;

  _nodes.insert_or_assign(node->id(), node);

  auto &ents = rd->renderables();

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
  //set_environment("C:\\Users\\t\\dev\\0\\FilaViewer/FilaViewer");
}
void FTScene::process(double timestamp)
{
#ifdef POINT_CLOUD_SUPPORT
  for (auto &iter : _pcs) {
    iter.second->_process(delta);
  }
#endif

  for (auto &iter : _nodes) {
    auto rd = iter.second->rdNode();
    if (!rd)
      continue;
    rd->update(timestamp);
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
