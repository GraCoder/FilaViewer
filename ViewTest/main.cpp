#include "FilaView/TWin.h"


int main() 
{
  auto win = TWin::create(0);
  //win->load_model("D:\\49_temp\\Unity\\UProject1\\Assets\\Mich-L-Resources\\Characters\\Ch_chicken_fx.fbx");
  win->load_model("D:\\03_Dat\\OneDrive\\03_Dat\\Models\\dragon\\dragon.fbx", 10);
  //win->load_model("D:\\03_Dat\\OneDrive\\03_Dat\\Models\\dragon\\dragon_action.fbx", 10);
  //win->load_model("C:\\Users\\t\\OneDrive\\03_Dat\\Models\\UnityBall.fbx");
  win->exec();
}