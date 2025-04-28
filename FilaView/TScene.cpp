#include "TScene.h"

#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Frustum.h>

#include "TDef.h"
#include "TView.h"

#include "intern/FTScene.h"
#include "intern/FTView.h"

#ifdef POINT_CLOUD_SUPPORT
#include "PCMesh.h"
#include "PCTile.h"
#include "PCDiv.h"
#include "PCNode.h"
#include "PCLoader.h"
#include "PCDispatch.h"
#endif

FT_DOWNCAST(TScene)
FT_DOWNCAST(TView)

std::shared_ptr<TScene> TScene::create()
{
  auto scene = std::make_shared<FTScene>();
  return scene;
}

TScene::TScene() = default;

TScene::~TScene() 
{ 
#ifdef POINT_CLOUD_SUPPORT
  _dispacher.reset(); 
#endif
}

void TScene::add_node(const std::shared_ptr<Node> &node) 
{
  downcast(this)->_add_node(node);
}

void TScene::add_pointcloud(const std::shared_ptr<fpc::PCNode> &node)
{
#ifdef POINT_CLOUD_SUPPORT
  if (!_dispacher) {
    _dispacher = fpc::PCDispatch::create();
  }

  _pcs[node->file()] = node;

  node->set_scene(this);

  _dispacher->regist_loader(std::hash<std::string>{}(node->file()), node->loader());
#endif
}

void TScene::get_pos(tg::vec4d planes[5])
{
#ifdef POINT_CLOUD_SUPPORT
  for (auto iter : _pcs) {
    fpc::PCDiv *div = nullptr;
    auto &divs = iter.second->loader()->divs();
    for (int i = 0; i < divs.size(); i++) {
      auto bb = divs[i]->get_aabb();
      bool out = false;
      float radius = bb.radius();
      for (int j = 0; j < 5; j++) {
        auto dis = tg::dot(planes[j], tg::vec4d(bb.center(), 1)) + radius;
        if (dis < 0) {
          out = true;
          break;
        }
      }
      if (out)
        continue;

      div = divs[i].get();
      auto p = div->get_pos(planes);
      if (p) {
        auto ps = std::format("x:{:.2f} y:{:.2f} z:{:.2f}\n", p->x(), p->y(), p->z());
        printf(ps.c_str());

        auto bd = div->get_aabb();
        downcast(this)->show_box(bd);
        return;
      }
    }
  }
#endif
}

void TScene::release()
{
#ifdef POINT_CLOUD_SUPPORT
  _pcs.clear();
  if(_dispacher)
    _dispacher->clean();
#endif
}

void TScene::dispatch()
{
#ifdef POINT_CLOUD_SUPPORT
  using namespace filament;

  if (!_dispacher)
    return;

  auto f2v = [](math::float3 &f) -> tg::vec3 { return tg::vec3(f.x, f.y, f.z); };
  math::float3 eye, pos, up;
  auto manip = downcast(this)->view()->manip();
  if (manip) {
    manip->getLookAt(&eye, &pos, &up);
    _dispacher->set_viewpoint(f2v(eye), f2v(pos), f2v(up));
  }

  auto &cam = downcast(this)->view()->view()->getCamera();
  math::float4 planes[6];
  cam.getFrustum().getNormalizedPlanes(planes);
  {
    std::vector<tg::vec4d> ps;
    //auto m = tg::lookat(f2v(eye), f2v(pos), f2v(up));
    //m = m.transpose();
    for (int i = 0; i < 6; i++) {
      tg::vec4 p = {planes[i].x, planes[i].y, planes[i].z, planes[i].w};
      //  p = m * -p;
      ps.push_back(-p);
    }

    _dispacher->set_frustum(ps);
  }

  _dispacher->dispatch();
#endif
}
