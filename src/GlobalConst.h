#ifndef HG_GLOBALCONST_H
#define HG_GLOBALCONST_H
namespace hg {
    inline static const int FRAMERATE = 60;
    inline static const int GUY_SPEED = 200;
    inline static const int GUY_JUMP_SPEED_DEFAULT = 270;
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

    inline std::vector<const char*> const validationLayers{
        "VK_LAYER_LUNARG_standard_validation"
    };
#ifdef NDEBUG
    inline bool const enableValidationLayers = false;
#else
    inline bool const enableValidationLayers = true;
#endif
}
#endif //HG_GLOBALCONST_H