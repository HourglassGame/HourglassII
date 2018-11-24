#ifndef HG_GLOBALCONST_H
#define HG_GLOBALCONST_H
namespace hg {
    inline static const int FRAMERATE = 60;
    inline static const int GUY_SPEED = 200;
    inline static const int GUY_JUMP_SPEED_DEFAULT = 300;
    inline static const double UI_DIVIDE_X = 17.0;
    inline static const double UI_DIVIDE_Y = 73.0;
    inline static const double G_TIME_Y = 0.0625;
    inline static const double G_TIME_HEIGHT = 10.*0.0625;
    inline static const double P_TIME_Y = 12.*0.0625;
    inline static const double P_TIME_HEIGHT = 3.*0.0625;
    inline static const double TIMELINE_PAD_X = 0.02;
    inline static const int WINDOW_DEFAULT_X = 1260;
    inline static const int WINDOW_DEFAULT_Y = 840;
}
#endif //HG_GLOBALCONST_H