#include "Explosion.h"

namespace hg {
Explosion::Explosion(
	int x, int y,
	int width,
	int height,
	int radius,
	int radiusMax,
	int radiusGrow,
	TimeDirection timeDirection):
		x(x), y(y),
		width(width), height(height),
		radius(radius), radiusMax(radiusMax), radiusGrow(radiusGrow),
		timeDirection(timeDirection)
{
}

Explosion::Explosion(Explosion const &o, hg::TimeDirection timeDirection) :
		x(o.x), y(o.y),
		width(o.width), height(o.height),
		radius(o.radius), radiusMax(o.radiusMax), radiusGrow(o.radiusGrow),
		timeDirection(timeDirection)
{
}

std::ostream &operator<<(std::ostream &str, Explosion const &e)
{
	return 
	  str << "{"
		  <<   "x=" << e.x
		  << ", y=" << e.y
		  << ", width=" << e.width
		  << ", height=" << e.height
		  << ", radius=" << e.radius
		  << ", radiusMax=" << e.radiusMax
		  << ", radiusGrow=" << e.radiusGrow
		  << ", timeDirection=" << e.timeDirection
		  << "}";
}
}//namespace hg
