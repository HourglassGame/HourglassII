#ifndef HG_MAIN_MENU_SCENE_H
#define HG_MAIN_MENU_SCENE_H
#include "Scene.h"
#include "variant.h"
#include "RenderWindow.h"
#include "VulkanEngine.h"
namespace hg {
    variant<RunALevel_tag, RunAReplay_tag, Exit_tag> run_main_menu(hg::RenderWindow &window, VulkanEngine &vulkanEng);
}

#endif //HG_MAIN_MENU_SCENE_H
