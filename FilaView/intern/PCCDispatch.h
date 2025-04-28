#pragma once

#include "PCDispatch.h"

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

namespace fpc {

class PCCDispatch : public PCDispatch {
public:
  PCCDispatch();

  ~PCCDispatch();

  void start();

  void shutdown();

  void dispatch();

  void load_tiles();

private:

  void load_tile_procedure(CamInfo c, const std::vector<std::shared_ptr<PCDiv>> &divs);

private:

  void routine();

  bool div_visible(tg::boundingbox &bb, const CamInfo &cam);

private:

  bool _run = true;

  std::thread _thread;
  asio::io_context _context;
  asio::steady_timer _timer;
};

} // namespace fpc