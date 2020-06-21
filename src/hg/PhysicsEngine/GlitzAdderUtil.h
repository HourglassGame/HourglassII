#ifndef HG_GLITZ_ADDER_UTIL_H
#define HG_GLITZ_ADDER_UTIL_H
#include "hg/mt/std/string"
namespace hg {


mt::std::string getBoxImage(BoxType boxType, TimeDirection timeDiretion) {
	if (timeDiretion == TimeDirection::FORWARDS) {
		if (boxType == BoxType::BALLOON) {
			return mt::std::string("global.balloon");
		}
		return mt::std::string("global.box");
	}
	else {
		if (boxType == BoxType::BALLOON) {
			return mt::std::string("global.balloon_r");
		}
		return mt::std::string("global.box_r");
	}
}

}//namespace hg
#endif //HG_GLITZ_ADDER_UTIL_H
