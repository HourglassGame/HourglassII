#ifndef HG_GUY_OUTPUT_INFO_H
#define HG_GUY_OUTPUT_INFO_H
#include <cstddef>
#include "hg/TimeEngine/ArrivalDepartures/TimeDirection.h"
#include "hg/TimeEngine/ArrivalDepartures/Ability.h"
#include "hg/mt/std/map"
namespace hg {
class GuyOutputInfo final {
public:
	explicit GuyOutputInfo(
		std::size_t index,
		TimeDirection timeDirection,
		Pickups pickups,
		BoxType boxCarrying,
		TimeDirection boxCarryDirection,
		int x,
		int y)
	: index(index),
	  timeDirection(timeDirection),
	  pickups(std::move(pickups)),
	  boxCarrying(boxCarrying),
	  boxCarryDirection(boxCarryDirection),
	  x(x),
	  y(y)
	{}
	std::size_t getIndex() const { return index; }
	TimeDirection getTimeDirection() const { return timeDirection; }
	Pickups const &getPickups() const { return pickups; }
	BoxType getBoxCarrying() const { return boxCarrying; }
	TimeDirection getBoxCarryDirection() const { return boxCarryDirection; }
	int getX() const { return x; }
	int getY() const { return y; }
private:
	std::size_t index;
	TimeDirection timeDirection;
	Pickups pickups;
	BoxType boxCarrying;
	TimeDirection boxCarryDirection;
	int x;
	int y;
};
}//namespace hg
#endif //HG_GUY_OUTPUT_INFO_H
