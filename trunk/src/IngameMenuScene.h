#ifndef HG_INGAME_MENU_SCENE_H
#define HG_INGAME_MENU_SCENE_H

namespace hg {
struct RenderWindow;
struct LoadedLevel;
/*
//Notes:
      Pauses game
      Gives timeline viewer
      Allows step-by-step play, and replay modification
  Resume
  Restart
  Take control (when in replay mode)
  Load Replay
  Save Replay
  Options
  Exit
*/
void run_ingame_menu(hg::RenderWindow &window, LoadedLevel &level);
}
#endif //HG_INGAME_MENU_SCENE_H
