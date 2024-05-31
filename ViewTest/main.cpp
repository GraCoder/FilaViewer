#include "FilaView/TWin.h"


int main() 
{
  auto win = TWin::create(0);
  //win->load_model("D:\\49_temp\\Unity\\UProject1\\Assets\\Mich-L-Resources\\Characters\\Ch_chicken_fx.fbx");
  //win->load_model("C:\\Users\\t\\OneDrive\\03_Dat\\Models\\dragon\\Dragon 2.5_fbx.fbx", true);
  //win->load_model("C:\\Users\\t\\OneDrive\\03_Dat\\Models\\UnityBall.fbx");
  win->load_model(
    "D:\\49_temp\\Unity\\UProject1\\Assets\\Mich-L-Resources\\Characters\\Rig_ch_mich-l01.fbx");
  win->exec();
}