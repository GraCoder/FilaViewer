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
#include "res/ibl.h"
#include "res/skybox.h"

#include "mesh/Cube.h"
#include "mesh/Sphere.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

Sphere *_sphere = 0;
Cube *_cube = 0;

#include "imgui/imgui.h"

#ifdef POINT_CLOUD_SUPPORT
#include "PCDispatch.h"
#include "PCNode.h"
#endif

#include "intern/mesh/RDShape.h"
#include "intern/RD_Gltf.h"
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

} // namespace

FTScene::FTScene()
  : TScene()
{
}

FTScene::~FTScene()
{
  if (_sphere)
    delete _sphere;
  if (_cube)
    delete _cube;

  if (_engine) {
    _engine->destroy(_default_material);
    _engine->destroy(_basic_material);
    _engine->destroy(_scene);
  }
}

void FTScene::show_box(const tg::boundingbox &box) 
{
#if 0
  if (!_cube)
    return;

  auto &tcm = _engine.getTransformManager();
  auto ti = tcm.getInstance(_cube->getWireFrameRenderable());
  tg::vec3 ct = box.center(), sz = box.max() - box.min();
  auto fm = math::mat4f::translation<float>(math::float3(ct.x(), ct.y(), ct.z())) *
            math::mat4f::scaling<float>(math::float3(sz.x(), sz.y(), sz.z()));
  tcm.setTransform(ti, fm);
#endif
}

//void FTScene::add_test_scene()
//{
//  {
//    auto sphere = new Sphere(*_engine, _default_material);
//    _sphere = sphere;
//    auto material = sphere->getMaterialInstance();
//    material->setParameter("baseColor", RgbType::sRGB, math::float3(1, 0, 0));
//    material->setParameter("metallic", 0);
//    material->setParameter("roughness", 1);
//    material->setParameter("reflectance", 0);
//    _scene->addEntity(sphere->getSolidRenderable());
//  }
//
//  if (1) {
//    auto cube = new Cube(*_engine, _basic_material, math::float3(0, 10, 0));
//    _cube = cube;
//    auto ce = cube->getWireFrameRenderable();
//    _scene->addEntity(ce);
//    auto &tm = _engine->getTransformManager();
//    auto s = filament::math::mat4::scaling(20);
//    tm.setTransform(tm.getInstance(ce), s);
//  }
//}

void FTScene::set_environment(const std::string &img_path, bool filter)
{
  using namespace filament;

  auto light = utils::EntityManager::get().create();
  LightManager::Builder(LightManager::Type::SUN)
    .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
    .intensity(110000)
    .direction({-1.0, 0, 0})
    .sunAngularRadius(1.9f)
    .castShadows(false)
    .sunHaloSize(10.0f)
    .sunHaloFalloff(80.f)
    .build(*_engine, light);
  _scene->addEntity(light);

  auto create_ktx = [](const std::string &path) {
    using namespace std;
    ifstream file(path, ios::binary);
    vector<uint8_t> contents((istreambuf_iterator<char>(file)), {});
    return new image::Ktx1Bundle(contents.data(), contents.size());
  };

  image::Ktx1Bundle *skybox_ktx = nullptr;
  if(img_path.empty())
    skybox_ktx = new image::Ktx1Bundle(skybox, skybox_length);
  else
    skybox_ktx = create_ktx(img_path + "_skybox.ktx");
  _skybox_tex = ktxreader::Ktx1Reader::createTexture(_engine, skybox_ktx, true);
  new image::Ktx1Bundle((uint8_t *)skybox, skybox_length);
  _skybox = Skybox::Builder()
    .environment(_skybox_tex)
    .showSun(true)
    .build(*_engine);
  _scene->setSkybox(_skybox);

  image::Ktx1Bundle *ibl_ktx = nullptr;
  if(img_path.empty())
    ibl_ktx = new image::Ktx1Bundle(::ibl, ibl_length);
  else
    ibl_ktx = create_ktx(img_path + "_ibl.ktx");
  auto irrtex = ktxreader::Ktx1Reader::createTexture(_engine, ibl_ktx, true);
  auto irrlight = IndirectLight::Builder()
    .reflections(irrtex)
    //.intensity(30000)
    .build(*_engine);
  _scene->setIndirectLight(irrlight);
}

int FTScene::load_model(const std::string &file, float size)
{
  if (file.ends_with("gltf") || file.ends_with("glb")) {
    auto node = std::make_shared<GltfNode>(file);
    std::unique_lock<std::mutex> lock(_mutex);
    _tasks.push(std::bind(
      [this](const std::shared_ptr<GltfNode> &node) {
        auto rd = static_cast<RD_Gltf *>(node->get_rd(true).get());
        rd->build(_engine);
        add_node(node);
      },
      node));
    return node->id();
  }

  auto node = std::make_shared<ModelNode>(file);

  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push([this, node]() { 
    auto rd = static_cast<RD_Model *>(node->get_rd(true).get());
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
  auto &rds = iter->second->get_rd()->get_renderables();
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
  }

  if (!node)
    return -1;

  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push(std::bind([this, node]() {
    auto rd = static_cast<RDShape *>(node->get_rd().get());
    rd->build(_engine, _default_material);
    _add_node(node);
  }));
  return node->id();
}

void FTScene::_add_node(const std::shared_ptr<Node> &node) 
{
  auto &rd = node->get_rd();
  if (!rd) return;

  _nodes.insert_or_assign(node->id(), node);

  auto &ents = rd->get_renderables();

  for (auto ent : ents) {
    _scene->addEntity(utils::Entity::import(ent));
  }
}

void FTScene::realize(filament::Engine *engine)
{
  if (_realized)
    return;

  _realized = true;

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
#ifdef POINT_CLOUD_SUPPORT
  for (auto &iter : _pcs) {
    iter.second->_process(delta);
  }
#endif

  for (auto &iter : _nodes) {
    auto &rd = iter.second->get_rd();
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

void FTScene::gui(filament::Engine *, filament::View *)
{
#ifdef POINT_CLOUD_SUPPORT
  ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

  ImGui::Begin("Point Cloud");
  if (ImGui::Button("Add PC")) {
    auto pc = std::make_shared<fpc::PCNode>();
    if (pc->load_file("D:\\07_Temp\\tiny\\test.tpcd")) {
      add_pc(pc);
      auto ab = pc->get_aabb();
      view()->set_pivot(ab.center(), 200);
    }
  }
  if (ImGui::Button("Calculate Point")) {
    _point_count = 0;
    for (auto iter : _pcs)
      _point_count += iter.second->point_count();
  }
  ImGui::Text("Point Count: %d", _point_count);

  ImGui::End();
#endif
}
