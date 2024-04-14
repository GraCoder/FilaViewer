#include "PCCDispatch.h"

#include <chrono>
#include <queue>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/error.hpp>

#include "PCDiv.h"
#include "PCTile.h"
#include "PCLoader.h"

#include <pcd/PCBlk.h>

namespace fpc {

inline PCCDispatch *dcast(PCDispatch *ptr) { return static_cast<PCCDispatch *>(ptr); }

std::unique_ptr<PCDispatch> PCDispatch::create()
{
  auto ptr = std::make_unique<PCCDispatch>();
  return ptr;
}

void PCDispatch::dispatch() { dcast(this)->dispatch(); }

PCCDispatch::PCCDispatch()
  : _timer(_context)
{
  start();
}

PCCDispatch::~PCCDispatch() { shutdown(); }

void PCCDispatch::start() { _thread = std::thread(std::bind(&PCCDispatch::routine, this)); }

void PCCDispatch::shutdown()
{
  _run = false;
  _context.stop();
  if (_thread.joinable())
    _thread.join();
}

void PCCDispatch::dispatch()
{
  _timer.cancel();
  _timer.expires_after(std::chrono::milliseconds(800));
  _timer.async_wait([this](const asio::error_code &ec) {
    if (ec)
      return;
    load_tiles();
  });
}

void PCCDispatch::load_tiles()
{
  std::vector<std::shared_ptr<PCDiv>> divs;
  for (auto iter = _loaders.begin(); iter != _loaders.end(); iter++) {
    auto &pdiv = iter->second->divs();
    divs.insert(divs.end(), pdiv.begin(), pdiv.end());
  }

  if (!divs.empty())
    asio::dispatch(_context, std::bind(&PCCDispatch::load_tile_procedure, this, _cam_info, std::move(divs)));
}

void PCCDispatch::load_tile_procedure(CamInfo cam, const std::vector<std::shared_ptr<PCDiv>> &divs)
{
  // 1000 700 500 300 100 0
  // 0    1   2   3   4  5

  _frame++;

  struct Tile {
    PCDiv *div = nullptr;
    int lev = -1;

    float life = 0;

    bool operator<(const Tile &other) const { return life > other.life; }
  };
  std::priority_queue<Tile> tile_queue;

  auto fun = [](float dis, int lev, bool visible) -> float {
    float l = dis * 200;
    if (!visible)
      l += 60000;

    return l + lev * 10000;
  };

  float minlen = FLT_MAX;
  float maxlen = 0;

  for (int i = 0; i < divs.size(); i++) {
    auto &div = divs[i];
    auto ab = div->get_aabb();
    float len = 1000;
    len = tg::length<float>(ab.center() - cam.pos);
    if (len < minlen) {
      minlen = std::min(len, minlen);
      _near_tile = div.get();
    }
    if (len > maxlen) {
      maxlen = std::max(len, maxlen);
      _far_tile = div.get();
    }

    constexpr float a = 5.84336057e-06;
    constexpr float b = -1.08473462e-02;
    constexpr float c = 5.00160231e+00;
    int lev = a * len * len + b * len + c;
    lev = std::min(std::max(0, lev), tpc::max_level);

    bool visible = div_visible(ab, cam);

    for (int j = 1; j <= div->level(); j++) {
      float p = fun(len, j, visible && (j <= lev));
      tile_queue.push({div.get(), j, p});
    }

    if (!visible)
      continue;

    lev = std::min<int>(div->blk()->level() - 1, lev);

    for (int j = div->level() + 1; j <= lev; j++) {
      float p = fun(len, j, visible);
      tile_queue.push({div.get(), j, p});

      auto blk = div->blk();
      assert(blk->pos_offset()[j + 1] > 0);
    }
  }

  uint32_t count = 0;
  while (!tile_queue.empty()) {
    auto &unit = tile_queue.top();
    auto div = unit.div;
    int lev = unit.lev;

    count += std::min<int>(div->blk()->count(lev), PCTile::max_points);

    if (count > 3e7 || unit.life > 1e5)
      break;

    tile_queue.pop();

    auto tile = div->get_tile(lev);
    if (!tile) {
      auto blk = div->blk();
      tile = std::make_shared<PCTile>(div, lev);
      div->add_tile(tile);
    }

    auto loader = div->loader();

    loader->load_tile(div, lev, _frame);
  }

  while (!tile_queue.empty()) {
    auto &unit = tile_queue.top();
    auto div = unit.div;
    auto tile = div->get_tile(unit.lev);

    if (tile)
      count += tile->count();

    if (count > 6e7)
      break;

    tile_queue.pop();

    if (!tile)
      continue;

    auto loader = div->loader();
    loader->unmount_tile(div, tile);
  }

  while (!tile_queue.empty()) {
    auto &unit = tile_queue.top();
    auto div = unit.div;
    auto tile = div->get_tile(unit.lev);

    if (tile) {
      auto loader = div->loader();
      loader->delete_tile(div, tile);

      div->remove_tile(unit.lev);
      assert(unit.lev == tile->level());
    }

    tile_queue.pop();
  }
}

void PCCDispatch::routine()
{
  do {
    _context.restart();
    _context.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (_run);
}

bool PCCDispatch::div_visible(tg::boundingbox &bb, const CamInfo &cam)
{
  for (int i = 0; i < cam.frustum.size(); i++) {
    auto &plane = cam.frustum[i];
    float dis = tg::dot<double, 4>(plane, tg::vec4d(bb.center(), 1));
    dis += bb.radius();
    if (dis < 0)
      return false;
  }

  return true;
}

} // namespace fpc