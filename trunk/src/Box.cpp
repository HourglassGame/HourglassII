#include "Box.h"

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

Box::Box(Box const& o, hg::TimeDirection timeDirection) :
        x_(o.x_), y_(o.y_),
        xspeed_(o.xspeed_), yspeed_(o.yspeed_),
        size_(o.size_),
        illegalPortal_(o.illegalPortal_),
        arrivalBasis_(o.arrivalBasis_),
        timeDirection_(timeDirection)
{
}

bool Box::operator==(Box const& o) const
{
    return x_ == o.x_
        && y_ == o.y_
        && xspeed_ == o.xspeed_
        && yspeed_ == o.yspeed_
        && size_ == o.size_
        && illegalPortal_ == o.illegalPortal_
        && arrivalBasis_ == o.arrivalBasis_
        && timeDirection_ == o.timeDirection_;
}

bool Box::operator<(const Box& o) const
{
    if (x_ == o.x_) {
        if (y_ == o.y_) {
            if (xspeed_ == o.xspeed_) {
                if (yspeed_ == o.yspeed_) {
                    if (size_ == o.size_) {
                        if (illegalPortal_ == o.illegalPortal_) {
                            if (arrivalBasis_ == o.arrivalBasis_) {
                                return timeDirection_ < o.timeDirection_;
                            }
                            return arrivalBasis_ < o.arrivalBasis_;
                        }
                        return illegalPortal_ < o.illegalPortal_;
                    }
                    return size_ < o.size_;
                }
                return yspeed_ < o.yspeed_;
            }
            return xspeed_ < o.xspeed_;
        }
        return y_ < o.y_;
    }
    return x_ < o.x_;
}
std::ostream& operator<<(std::ostream& str, Box const& b)
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