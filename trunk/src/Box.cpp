#include "Box.h"

#include <boost/tuple/tuple_comparison.hpp>

namespace hg {
Box::Box(
    int x, int y,
    int xspeed, int yspeed,
    int size,
    int illegalPortal,
    int arrivalBasis,
    TimeDirection timeDirection):
        x(x), y(y),
        xspeed(xspeed), yspeed(yspeed),
        size(size),
        illegalPortal(illegalPortal),
        arrivalBasis(arrivalBasis),
        timeDirection(timeDirection)
{
}

Box::Box(Box const &o, hg::TimeDirection timeDirection) :
        x(o.x), y(o.y),
        xspeed(o.xspeed), yspeed(o.yspeed),
        size(o.size),
        illegalPortal(o.illegalPortal),
        arrivalBasis(o.arrivalBasis),
        timeDirection(timeDirection)
{
}

bool Box::operator==(Box const &o) const
{
    return asTie() == o.asTie();
}

bool Box::operator<(Box const &o) const
{
    return asTie() < o.asTie();
}



std::ostream &operator<<(std::ostream &str, Box const &b)
{
    return 
      str << "{"
          <<   "x=" << b.x
          << ", y=" << b.y
          << ", xspeed=" << b.xspeed
          << ", yspeed=" << b.yspeed
          << ", size=" << b.size
          << ", illegalPortal=" << b.illegalPortal
          << ", arrivalBasis=" << b.arrivalBasis
          << ", timeDirection=" << b.timeDirection
          << "}";
}
}//namespace hg
