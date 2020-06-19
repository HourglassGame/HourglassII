#ifndef HG_MUTATOR_AREA_H
#define HG_MUTATOR_AREA_H
#include "hg/TimeEngine/ArrivalDepartures/TimeDirection.h"
#include <ostream>
#include <tuple>
namespace hg {
class MutatorArea;
std::ostream &operator<<(std::ostream &os, MutatorArea const &toPrint);
class MutatorArea final {
	auto comparison_tuple() const -> decltype(auto)
	{
		return std::tie(
			x_,
			y_,
			width_,
			height_,
			xspeed_,
			yspeed_,
			collisionOverlap_,
			timeDirection_
		);
	}
public:
	MutatorArea(
		int x, int y,
		int width, int height,
		int xspeed, int yspeed,
		int collisionOverlap,
		TimeDirection timeDirection) :
			x_(x), y_(y),
			width_(width), height_(height),
			xspeed_(xspeed), yspeed_(yspeed),
			collisionOverlap_(collisionOverlap),
			timeDirection_(timeDirection) {}

	int getX()      const { return x_; }
	int getY()      const { return y_; }
	int getWidth()  const { return width_; }
	int getHeight() const { return height_; }
	int getXspeed() const { return xspeed_; }
	int getYspeed() const { return yspeed_; }
	int getCollisionOverlap() const { return collisionOverlap_; }
	TimeDirection 
		getTimeDirection() const { return timeDirection_; }

	friend std::ostream &operator<<(std::ostream &os, MutatorArea const &toPrint)
	{
		#define HG_MUTATOR_AREA_PRINT(obj) os << #obj << ": " << toPrint.obj << std::endl
		HG_MUTATOR_AREA_PRINT(x_);
		HG_MUTATOR_AREA_PRINT(y_);
		HG_MUTATOR_AREA_PRINT(width_);
		HG_MUTATOR_AREA_PRINT(height_);
		HG_MUTATOR_AREA_PRINT(xspeed_);
		HG_MUTATOR_AREA_PRINT(yspeed_);
		HG_MUTATOR_AREA_PRINT(collisionOverlap_);
		HG_MUTATOR_AREA_PRINT(timeDirection_);
		#undef HG_MUTATOR_AREA_PRINT
		return os;
	}
	bool operator==(MutatorArea const &o) const {
		return comparison_tuple() == o.comparison_tuple();
	}
private:
	int x_;
	int y_;
	int width_;
	int height_;
	int xspeed_;
	int yspeed_;
	int collisionOverlap_;
	TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_MUTATOR_AREA_H
