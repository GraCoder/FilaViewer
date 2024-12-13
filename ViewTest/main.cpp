#include "FilaView/TWin.h"
#include "FilaView/tvec.h"
#include "FilaView/tmath.h"

#include <string>

int main(int argc, char **argv) 
{
  auto win = TWin::create(0);

  if(1)
  {
    std::string ptr = getenv("OneDrive");
    // win->load_model("D:\\49_temp\\Unity\\UProject1\\Assets\\Mich-L-Resources\\Characters\\Ch_chicken_fx.fbx");
    //win->load_model((ptr + "\\03_Dat\\Models\\UnityBall.fbx").c_str());
    //win->load_model((ptr + "\\03_Dat\\Models\\dragon\\dragon.fbx").c_str(), 10);

    //win->load_model("C:\\Users\\t\\Downloads\\dragon.fbx", 20);

    // win->load_model("D:\\03_Dat\\OneDrive\\03_Dat\\Models\\dragon\\dragon.fbx", 10);
    // win->load_model("D:\\03_Dat\\OneDrive\\03_Dat\\Models\\dragon\\dragon_action.fbx", 10);

    //win->load_model((ptr + "\\03_Dat\\Models\\models\\windmill\\windmill.obj").c_str(), 10);
    //win->load_model((ptr + "\\03_Dat\\Models\\models\\knight\\KnightCharacter.gltf").c_str(), 10);
  }
  win->load_model("D:\\06_Test\\LearnOpenGL\\resources\\objects\\backpack\\backpack.obj");

  win->exec();
}