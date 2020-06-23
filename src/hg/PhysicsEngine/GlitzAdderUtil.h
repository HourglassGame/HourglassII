#ifndef HG_GLITZ_ADDER_UTIL_H
#define HG_GLITZ_ADDER_UTIL_H
#include "hg/LuaUtil/LuaUtilities.h"
#include "hg/mt/std/memory"
#include "hg/Util/Maths.h"
#include "hg/GlobalConst.h"

namespace hg {

inline unsigned asPackedColour(int const r, int const g, int const b) {
	return r << 24 | g << 16 | b << 8;
}

mt::std::string formatTime(int const frames) {
	std::stringstream ss; //TODO: Use appropriate allocator here!

	if (frames % hg::FRAMERATE == 0) {
		ss << floor(frames / static_cast<double>(hg::FRAMERATE)) << "s";
	}
	else {
		double const seconds = frames / static_cast<double>(hg::FRAMERATE);
		ss << static_cast<int>(floor(seconds)) << "." << static_cast<int>(floor(seconds * 10))%10 << "s";
	}
	//TODO: Use appropriate allocator here too!!
	auto s = ss.str();
	return { s.begin(), s.end() };
}

mt::std::string getBoxImage(BoxType boxType, TimeDirection timeDiretion) {
	if (timeDiretion == TimeDirection::FORWARDS) {
		if (boxType == BoxType::BALLOON) {
			return mt::std::string("global.balloon");
		}
		if (boxType == BoxType::BOMB) {
			return mt::std::string("global.bomb");
		}
		return mt::std::string("global.box");
	}
	else {
		if (boxType == BoxType::BALLOON) {
			return mt::std::string("global.balloon_r");
		}
		if (boxType == BoxType::BOMB) {
			return mt::std::string("global.bomb_r");
		}
		return mt::std::string("global.box_r");
	}
}

}//namespace hg
#endif //HG_GLITZ_ADDER_UTIL_H
