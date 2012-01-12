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

Box::Box(Box const& other, hg::TimeDirection timeDirection) :
        x_(other.x_), y_(other.y_),
        xspeed_(other.xspeed_), yspeed_(other.yspeed_),
        size_(other.size_),
        illegalPortal_(other.illegalPortal_),
        arrivalBasis_(other.arrivalBasis_),
        timeDirection_(timeDirection)
{
}

bool Box::operator==(Box const& other) const
{
    return x_ == other.x_
        && y_ == other.y_
        && xspeed_ == other.xspeed_
        && yspeed_ == other.yspeed_
        && size_ == other.size_
        && illegalPortal_ == other.illegalPortal_
        && arrivalBasis_ == other.arrivalBasis_
        && timeDirection_ == other.timeDirection_;
}

bool Box::operator<(const Box& other) const
{
    if (x_ == other.x_) {
        if (y_ == other.y_) {
            if (xspeed_ == other.xspeed_) {
                if (yspeed_ == other.yspeed_) {
                    if (size_ == other.size_) {
                        if (illegalPortal_ == other.illegalPortal_) {
                            if (arrivalBasis_ == other.arrivalBasis_) {
                                return timeDirection_ < other.timeDirection_;
                            }
                            return arrivalBasis_ < other.arrivalBasis_;
                        }
                        return illegalPortal_ < other.illegalPortal_;
                    }
                    return size_ < other.size_;
                }
                return yspeed_ < other.yspeed_;
            }
            return xspeed_ < other.xspeed_;
        }
        return y_ < other.y_;
    }
    return x_ < other.x_;
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
