#include "FilaView/TWin.h"
#include "FilaView/tvec.h"
#include "FilaView/tmath.h"

#include <spdlog/spdlog.h>

#include <string>

int main(int argc, char **argv) 
{
  auto m = tg::perspective(30, 1, float(0.1), float(1000));
  double l, r, b, t, n, f;
  tg::get_perspective(m,  b, t, n, f);

  spdlog::set_level(spdlog::level::debug);
  auto win = (fv::TWin*)IWin::create(0);

  if (argc == 1) {
    std::string ptr = getenv("OneDrive");
    // win->loadModel("D:\\49_temp\\Unity\\UProject1\\Assets\\Mich-L-Resources\\Characters\\Ch_chicken_fx.fbx");
    //win->loadModel((ptr + "\\03_Dat\\Models\\UnityBall.fbx").c_str());
    // win->loadModel((ptr + "\\03_Dat\\Models\\dragon\\dragon.fbx").c_str(), 10);

    //win->loadModel((ptr + "\\03_Dat\\Models\\models\\windmill\\windmill.obj").c_str(), 10);
    //win->loadModel((ptr + "\\03_Dat\\Models\\models\\knight\\KnightCharacter.gltf").c_str(), 10);
    //win->loadModel("D:\\06_Test\\LearnOpenGL\\resources\\objects\\backpack\\backpack.obj");
    win->loadModel("D:\\box.glb");
    //win->loadModel("D:\\plane.obj");
  } else {
    win->loadModel(argv[1], 10);
  }

  win->createOperators();
  win->setFlags(win->flags() | win->en_SetupGui);
  win->exec(false);
  fv::TWin::destroy(win);
}