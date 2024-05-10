#include "FTScene.h"

#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Material.h>
#include <filament/TransformManager.h>
#include <filament/RenderableManager.h>
#include <filament/LightManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>

#include "FTView.h"

#include "pcv_mat.h"
#include "mesh/Cube.h"
#include "mesh/Sphere.h"
#include "MeshAssimp.h"

Sphere *_sphere = 0;
Cube *_cube = 0;

#include "imgui/imgui.h"

#ifdef POINT_CLOUD_SUPPORT
#include "PCDispatch.h"
#include "PCNode.h"
#endif

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

} // namespace

FTScene::FTScene()
  : TScene()
{
  //view->set_gui_callback(std::bind(&FTScene::gui, this, std::placeholders::_1, std::placeholders::_2));
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

void FTScene::add_test_scene()
{
  {
    auto sphere = new Sphere(*_engine, _default_material);
    _sphere = sphere;
    auto material = sphere->getMaterialInstance();
    material->setParameter("baseColor", RgbType::sRGB, math::float3(1, 0, 0));
    material->setParameter("metallic", 0);
    material->setParameter("roughness", 1);
    material->setParameter("reflectance", 0);
    _scene->addEntity(sphere->getSolidRenderable());
  }

  if (1) {
    auto cube = new Cube(*_engine, _basic_material, math::float3(0, 10, 0));
    _cube = cube;
    _scene->addEntity(cube->getWireFrameRenderable());
  }
}

void FTScene::load_model(const std::string &file, bool normalize)
{
  FILE *fp = nullptr;
  if (file.ends_with("gltf")) {
    fp = fopen(file.c_str(), "rt");
  } else if (file.ends_with("glb")) {
    fp = fopen(file.c_str(), "rb");
  } else{
    assimp_load(file, normalize);
  }

  if (!fp)
    return;
  fseek(fp, 0, SEEK_END);
  auto sz = ftell(fp);
  std::vector<uint8_t> data(sz, 0);
  data.resize(sz);
  fseek(fp, 0, SEEK_SET);
  fread(data.data(), 1, sz, fp);
  fclose(fp);

  std::unique_lock<std::mutex> lock(_mutex);

  _tasks.push(std::bind(
    [this](const std::vector<uint8_t> &data) {
      auto mp = gltfio::createJitShaderProvider(_engine);
      auto loader = gltfio::AssetLoader::create({_engine, mp});
      auto asset = loader->createAsset(data.data(), data.size());

      gltfio::ResourceLoader res_loader({_engine});
      res_loader.loadResources(asset);

      asset->releaseSourceData();

      _scene->addEntities(asset->getLightEntities(), asset->getLightEntityCount());

      auto &rm = _engine->getRenderableManager();
      auto &tm = _engine->getTransformManager();

      {
        auto root = tm.getInstance(asset->getRoot());
        auto instance = asset->getInstance();
        auto aabb = instance ? instance->getBoundingBox() : asset->getBoundingBox();
        filament::math::mat4f transform;
        tm.setTransform(root, transform);
      }

      for (int i = 0, n = asset->getRenderableEntityCount(); i < n; i++) {
        auto ent = asset->getRenderableEntities()[i];

        _scene->addEntity(ent);

        auto ri = rm.getInstance(ent);
        rm.setScreenSpaceContactShadows(ri, true);
      }
    },
    std::move(data)));
}

void FTScene::realize(filament::Engine *engine) 
{
  if (_realized)
    return;

  _realized = true;

  _engine = engine;
  _scene = _engine->createScene();

  {
    auto basic_mtl = filament::Material::Builder()
                       .package(PCV_MAT_BASICMAT_DATA, PCV_MAT_BASICMAT_SIZE)
                       .build(*_engine);
    basic_mtl->setDefaultParameter("baseColor", RgbType::LINEAR, float3{0.8});
    _basic_material = basic_mtl;

    auto def_mtl = filament::Material::Builder()
                     .package(PCV_MAT_DEFAULTMAT_DATA, PCV_MAT_DEFAULTMAT_SIZE)
                     .build(*_engine);
    def_mtl->setDefaultParameter("baseColor", RgbType::LINEAR, float3{0.8});
    def_mtl->setDefaultParameter("metallic", 0.0f);
    def_mtl->setDefaultParameter("roughness", 0.4f);
    def_mtl->setDefaultParameter("reflectance", 0.5f);
    _default_material = def_mtl;

    auto trans_mtl = filament::Material::Builder()
                       .package(PCV_MAT_TRANSPARENTMAT_DATA, PCV_MAT_TRANSPARENTMAT_SIZE)
                       .build(*_engine);
    def_mtl->setDefaultParameter("baseColor", RgbaType::LINEAR, float4{0.8});
    def_mtl->setDefaultParameter("metallic", 0.0f);
    def_mtl->setDefaultParameter("roughness", 0.4f);
  }

  {
    math::float4 clr(0.2, 0.2, 0.2, 1);
    auto skybox = Skybox::Builder().color(clr).build(*_engine);
    _scene->setSkybox(skybox);
  }

  {
    auto light = utils::EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
      .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
      .intensity(110000)
      .direction({-1, -1, -1})
      .sunAngularRadius(1.9f)
      .castShadows(false)
      .build(*_engine, light);
    _scene->addEntity(light);
  }

  //add_test_scene();
}
void FTScene::process(float delta) 
{ 
#ifdef POINT_CLOUD_SUPPORT
  for (auto &iter : _pcs) {
    iter.second->_process(delta);
  }
 #endif

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
  if(ImGui::Button("Calculate Point")) {
    _point_count = 0;
    for (auto iter : _pcs)
      _point_count += iter.second->point_count(); 
  }
  ImGui::Text("Point Count: %d", _point_count);

  ImGui::End();
#endif
}

void FTScene::assimp_load(const std::string &file, bool normalize) 
{
  if (!_assimp)
    _assimp = std::make_unique<MeshAssimp>();

  if (!_assimp->load_assert(file.c_str()))
    return;

  std::unique_lock<std::mutex> lock(_mutex);
  _tasks.push([this, normalize]() { 
    _assimp->build_assert(_engine, _default_material, _transparent_material);

    for(auto ent : _assimp->renderables()){
      _scene->addEntity(ent);
    }

    if (normalize) {
      auto &tcm = _engine->getTransformManager();
      auto ti = tcm.getInstance(_assimp->root());
      auto &mi = _assimp->min_bound();
      auto &ma = _assimp->max_bound();
      auto m = ma - mi;
      auto f = std::max({m.x, m.y, m.z});
      tcm.setTransform(ti, mat4::scaling(2. / f) * mat4::translation((mi + ma) / 2.0));
    }
  });
}
