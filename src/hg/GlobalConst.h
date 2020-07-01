#ifndef HG_GLOBALCONST_H
#define HG_GLOBALCONST_H
#include "hg/Util/Maths.h"
#include <vector>
namespace hg {
    inline static const int FRAMERATE = 60;
    inline static const int BOMB_TIMER = 60 * 3;
    inline static const int GRAVITY = 17;
    inline static const int UP_GRAVITY = 17;
    inline static const int DOWN_GRAVITY = 17;
    inline static const int GUY_SPEED = 230;
    inline static const int GUY_HOR_ACCEL = 40;
    inline static const int GUY_HOR_SLOW = 70;
    inline static const int GUY_JUMP_SPEED_DEFAULT = 330;
    inline static const int GUY_JUMP_HOLD_MAX = 15;
    inline static const int GUY_JUMP_HOLD_MIN = 5;
    inline static const int GUY_JUMP_HOLD_SPEED = -16;
    inline static const int GUY_SUPPORT_JUMP_REQ = 6;
    inline static const int GUY_MAX_SUPPORTED = 60;
    inline static const int VERT_AIR_RESISTANCE = 22000000;

    inline static const int SPEED_OF_TIME_OVERRIDE = -1;
    inline static const int PARADOX_PRESSURE_MAX = 12000000;
    inline static const int PARADOX_PRESSURE_ADD_MIN = 14000;
    inline static const int PARADOX_PRESSURE_DECAY_BUILDUP = 100;
    inline static const int PARADOX_PRESSURE_DECAY_MAX = 8000;
    inline static const int PARADOX_PRESSURE_PER_FRAME = 100;
    //The game window is divided into 4 quadrants.
    //UI_DIVIDE_X and UI_DIVIDE_Y are the locations of the 2 lines that divide
    //the window into quadrants, as ratios of the window width/height.
    inline static const double UI_DIVIDE_X = 0.17;
    inline static const double UI_DIVIDE_Y = 0.73;
    inline static const double G_TIME_Y = 0.0625;
    inline static const double G_TIME_HEIGHT = 10.*0.0625;
    inline static const double P_TIME_Y = 12.*0.0625;
    inline static const double P_TIME_HEIGHT = 3.*0.0625;
    inline static const double TIMELINE_PAD_X = 0.02;
    inline static const int WINDOW_DEFAULT_X = 1260;
    inline static const int WINDOW_DEFAULT_Y = 840;

    inline int const MAX_FRAMES_IN_FLIGHT = 2;
    inline std::vector<const char*> const validationLayers{
        "VK_LAYER_KHRONOS_validation"
    };
    inline hg::vec3<float> const UI_TEXT_COLOR{ 100.f / 255.f, 100.f / 255.f, 200.f / 255.f };
    inline bool const enableValidationLayers =
#ifdef NDEBUG
        false
#else
        false
#endif
        ;

}
#endif //HG_GLOBALCONST_H