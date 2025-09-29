#include "FilaView/TWin.h"
#include "FilaView/tvec.h"
#include "FilaView/tmath.h"

#include <spdlog/spdlog.h>

#include <string>

int main(int argc, char **argv) 
{
  spdlog::set_level(spdlog::level::debug);
  auto win = (TWin*)IWin::create(0);

  if (argc == 1) {
    std::string ptr = getenv("OneDrive");
    // win->load_model("D:\\49_temp\\Unity\\UProject1\\Assets\\Mich-L-Resources\\Characters\\Ch_chicken_fx.fbx");
    //win->load_model((ptr + "\\03_Dat\\Models\\UnityBall.fbx").c_str());
    // win->load_model((ptr + "\\03_Dat\\Models\\dragon\\dragon.fbx").c_str(), 10);

    //win->load_model((ptr + "\\03_Dat\\Models\\models\\windmill\\windmill.obj").c_str(), 10);
    //win->load_model((ptr + "\\03_Dat\\Models\\models\\knight\\KnightCharacter.gltf").c_str(), 10);
    // win->load_model("D:\\06_Test\\LearnOpenGL\\resources\\objects\\backpack\\backpack.obj");
  } else {
    win->load_model(argv[1], 10);
  }

  win->create_operators();
  //win->set_flags(win->flags() | win->en_SetupGui);
  win->exec(false);
  TWin::destroy(win);
}