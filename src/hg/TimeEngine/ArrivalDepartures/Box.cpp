#include "Box.h"

namespace hg {
Box::Box(
	int x, int y,
	int xspeed, int yspeed,
	int width, int height,
	BoxType boxType,
	int state,
	int illegalPortal,
	int arrivalBasis,
	TimeDirection timeDirection):
		x(x), y(y),
		xspeed(xspeed), yspeed(yspeed),
		width(width), height(height),
		boxType(boxType), state(state),
		illegalPortal(illegalPortal),
		arrivalBasis(arrivalBasis),
		timeDirection(timeDirection)
{
}

Box::Box(Box const &o, hg::TimeDirection timeDirection) :
		x(o.x), y(o.y),
		xspeed(o.xspeed), yspeed(o.yspeed),
		width(o.width), height(o.height),
		boxType(o.boxType), state(o.state),
		illegalPortal(o.illegalPortal),
		arrivalBasis(o.arrivalBasis),
		timeDirection(timeDirection)
{
}

std::ostream &operator<<(std::ostream &str, Box const &b)
{
	return 
	  str << "{"
		  <<   "x=" << b.x
		  << ", y=" << b.y
		  << ", xspeed=" << b.xspeed
		  << ", yspeed=" << b.yspeed
		  << ", width=" << b.width
		  << ", height=" << b.height
		  << ", boxType=" << static_cast<int>(b.boxType)
		  << ", state=" << b.state
		  << ", illegalPortal=" << b.illegalPortal
		  << ", arrivalBasis=" << b.arrivalBasis
		  << ", timeDirection=" << b.timeDirection
		  << "}";
}
}//namespace hg
