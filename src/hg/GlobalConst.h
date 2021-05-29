#ifndef HG_GLOBALCONST_H
#define HG_GLOBALCONST_H
#include "hg/Util/Maths.h"
#include <vector>
namespace hg {
	inline static const uint32_t IMAGE_MAX_SETS =
		21 //Glitz Images MAGIC
		+ 2 * 2 * 2 * 2/*Wall Blocks*/
		+ 2 * 2 /*Wall Corners*/
		;//TODO; set this to match count of descriptors used in renderer

	// Setting this to true means that the current player always exists, however it means
	// that propagation in the future can happen, which is a little inconsistent.
	inline static const bool PROPAGATE_CHANGED_FUTURE_GUY_FRAME = true;

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

	inline static const int SPEED_OF_TIME_OVERRIDE = 24; // -1 disables the override.
	inline static const int SPEED_OF_TIME_FUTURE_DEFAULT = 0; // -1 defaults to the speed of time of the level.
	inline static const int PARADOX_PRESSURE_MAX = 25000000;
	inline static const int PARADOX_PRESSURE_ADD_MIN = 13400;
	inline static const int PARADOX_PRESSURE_DECAY_BUILDUP = 40;
	inline static const int PARADOX_PRESSURE_DECAY_MAX = 12000;
	inline static const int PARADOX_PRESSURE_PER_FRAME = 20;
	//The game window is divided into 4 quadrants.
	//UI_DIVIDE_X and UI_DIVIDE_Y are the locations of the 2 lines that divide
	//the window into quadrants, as ratios of the window width/height.
	inline static const double UI_DIVIDE_X = 0.17;
	inline static const double UI_DIVIDE_Y = 0.78;
	inline static const double G_TIME_Y = 0.095;
	inline static const double G_TIME_HEIGHT = 0.81;
	inline static const double P_TIME_Y = 12.*0.0625;
	inline static const double P_TIME_HEIGHT = 0.*0.0625; // 3.*0.0625;
	inline static const double TIMELINE_PAD_X = 0.02;
	inline static const int WINDOW_DEFAULT_X = 1260;
	inline static const int WINDOW_DEFAULT_Y = 840;

	inline static const float BUTTON_AREA_HEIGHT = 260.;
	inline static const float BUTTON_HEIGHT = 32.;
	inline static const float BUTTON_SPACING = 40.;
	inline static const float BUTTON_X = 0.12;
	inline static const float BUTTON_WIDTH = 0.76;
	inline static const float BUTTON_PAUSE_SPACING = 64.;
	inline static const float BUTTON_PAUSE_X = 0.27;
	inline static const float BUTTON_PAUSE_WIDTH = 0.46;

	// TODO move this somewhere better?
	inline static const float PRESS_FORCE_GUY = 0;
	inline static const float PRESS_LIGHT_BOX = 1;
	inline static const float PRESS_HEAVY_BOX = 2;

	inline int const MAX_FRAMES_IN_FLIGHT = 2;
	inline std::vector<const char*> const validationLayers{
		"VK_LAYER_KHRONOS_validation"
	};
	inline hg::vec3<float> const UI_TEXT_COLOR{ 100.f / 255.f, 100.f / 255.f, 200.f / 255.f };
	inline hg::vec3<float> const BUTTON_TEXT_COLOR{ 0.f / 255.f, 0.f / 255.f, 0.f / 255.f };
	inline hg::vec3<float> const BUTTON_ACTIVE{ 100.f / 255.f, 220.f / 255.f, 100.f / 255.f };
	inline hg::vec3<float> const BUTTON_INACTIVE{ 100.f / 255.f, 100.f / 255.f, 100.f / 255.f };
	inline bool const enableValidationLayers =
#ifdef NDEBUG
		false
#else
		false
#endif
		;

}
#endif //HG_GLOBALCONST_H