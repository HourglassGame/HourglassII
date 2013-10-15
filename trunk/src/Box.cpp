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
        x_(x), y_(y),
        xspeed_(xspeed), yspeed_(yspeed),
        size_(size),
        illegalPortal_(illegalPortal),
        arrivalBasis_(arrivalBasis),
        timeDirection_(timeDirection)
{
}

Box::Box(Box const &o, hg::TimeDirection timeDirection) :
        x_(o.x_), y_(o.y_),
        xspeed_(o.xspeed_), yspeed_(o.yspeed_),
        size_(o.size_),
        illegalPortal_(o.illegalPortal_),
        arrivalBasis_(o.arrivalBasis_),
        timeDirection_(timeDirection)
{
}

bool Box::operator==(Box const &o) const
{
    return asTie() == o.asTie();
}

bool Box::operator<(const Box &o) const
{
    return asTie() < o.asTie();
}

boost::tuple<
        int const &, int const &, int const &, int const &, int const &,
        int const &, int const &,
        TimeDirection const &>
Box::asTie() const {
    return boost::tie(
            x_, y_, xspeed_, yspeed_, size_,
            illegalPortal_, arrivalBasis_,
            timeDirection_);
}

std::ostream &operator<<(std::ostream &str, Box const &b)
{
    return 
      str << "{"
          <<   "x=" << b.x_
          << ", y=" << b.y_
          << ", xspeed=" << b.xspeed_
          << ", yspeed=" << b.yspeed_
          << ", size=" << b.size_
          << ", illegalPortal=" << b.illegalPortal_
          << ", arrivalBasis=" << b.arrivalBasis_
          << ", timeDirection=" << b.timeDirection_
          << "}";
}
}//namespace hg
