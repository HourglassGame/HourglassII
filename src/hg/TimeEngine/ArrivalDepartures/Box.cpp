#include "Box.h"

#include <boost/tuple/tuple_comparison.hpp>

namespace hg {
Box::Box(
	int x, int y,
	int xspeed, int yspeed,
	int width, int height,
	BoxType boxType,
	int illegalPortal,
	int arrivalBasis,
	TimeDirection timeDirection):
		x(x), y(y),
		xspeed(xspeed), yspeed(yspeed),
		width(width), height(height),
		boxType(boxType),
		illegalPortal(illegalPortal),
		arrivalBasis(arrivalBasis),
		timeDirection(timeDirection)
{
}

Box::Box(Box const &o, hg::TimeDirection timeDirection) :
		x(o.x), y(o.y),
		xspeed(o.xspeed), yspeed(o.yspeed),
		width(o.width), height(o.height),
		boxType(o.boxType),
		illegalPortal(o.illegalPortal),
		arrivalBasis(o.arrivalBasis),
		timeDirection(timeDirection)
{
}

bool Box::operator==(Box const &o) const
{
	return comparison_tuple() == o.comparison_tuple();
}

bool Box::operator<(Box const &o) const
{
	return comparison_tuple() < o.comparison_tuple();
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
		  << ", illegalPortal=" << b.illegalPortal
		  << ", arrivalBasis=" << b.arrivalBasis
		  << ", timeDirection=" << b.timeDirection
		  << "}";
}
}//namespace hg
