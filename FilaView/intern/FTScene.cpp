#include "FTScene.h"

#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include "FTView.h"

#include "mesh/Cube.h"
#include "mesh/Sphere.h"
#include "pcv_mat.h"

#include "imgui/imgui.h"

#include "PCDispatch.h"
#include "PCNode.h"

Sphere *_sphere = 0;
Cube *_cube = 0;

using namespace filament;

FTScene::FTScene(FTView *view)
  : TScene()
  , _engine(view->engine())
{
  _view = view;
  _scene = _engine.createScene();
  view->view()->setScene(_scene);

  {
    math::float4 clr(0, 0.125, 0.25, 1.0);
    // clr.xyz = math::float3(1, 1, 1);
    auto skybox = Skybox::Builder().color(clr).build(_engine);
    _scene->setSkybox(skybox);
  }

  {
    auto light = utils::EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
      .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
      .intensity(110000)
      .direction({0.7, -1, -0.8})
      .sunAngularRadius(1.9f)
      .castShadows(false)
      .build(_engine, light);
    _scene->addEntity(light);
  }

  {
    _basic_material =
      filament::Material::Builder().package(PCV_MAT_BASICMAT_DATA, PCV_MAT_BASICMAT_SIZE).build(_engine);
    _default_material =
      filament::Material::Builder().package(PCV_MAT_DEFAULTMAT_DATA, PCV_MAT_DEFAULTMAT_SIZE).build(_engine);
  }

  //{
  //  auto sphere = new Sphere(*_engine, TApp::app()->default_mat());
  //  _sphere = sphere;
  //  auto material = sphere->getMaterialInstance();
  //  material->setParameter("baseColor", RgbType::sRGB, math::float3(1, 0, 0));
  //  material->setParameter("metallic", 0);
  //  material->setParameter("roughness", 1);
  //  material->setParameter("reflectance", 0);
  //  _scene->addEntity(sphere->getSolidRenderable());
  //}

  if(1)
  {
    auto cube = new Cube(_engine, _basic_material, math::float3(0, 10, 0));
    _cube = cube;
    _scene->addEntity(cube->getWireFrameRenderable());
  }

  view->set_gui_callback(std::bind(&FTScene::gui, this, std::placeholders::_1, std::placeholders::_2));
}

FTScene::~FTScene()
{
  if (_sphere)
    delete _sphere;
  if (_cube)
    delete _cube;

  if (_default_material)
    _engine.destroy(_default_material);

  if (_basic_material)
    _engine.destroy(_basic_material);

  _engine.destroy(_scene);
}

void FTScene::show_box(const tg::boundingbox &box) 
{
  if (!_cube)
    return;

  auto &tcm = _engine.getTransformManager();
  auto ti = tcm.getInstance(_cube->getWireFrameRenderable());
  tg::vec3 ct = box.center(), sz = box.max() - box.min();
  auto fm = math::mat4f::translation<float>(math::float3(ct.x(), ct.y(), ct.z())) *
            math::mat4f::scaling<float>(math::float3(sz.x(), sz.y(), sz.z()));
  tcm.setTransform(ti, fm);
}

void FTScene::gui(filament::Engine *, filament::View *)
{
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
}
