#include "Box.h"

namespace hg {
Box::Box(
    int x, int y,
    int xspeed, int yspeed,
    int size,
    int illegalPortal,
    int relativeToPortal,
    TimeDirection timeDirection):
        x_(x), y_(y),
        xspeed_(xspeed), yspeed_(yspeed),
        size_(size),
        illegalPortal_(illegalPortal),
        relativeToPortal_(relativeToPortal),
        timeDirection_(timeDirection)
{
}

Box::Box(Box const& other, hg::TimeDirection timeDirection) :
        x_(other.x_), y_(other.y_),
        xspeed_(other.xspeed_), yspeed_(other.yspeed_),
        size_(other.size_),
        illegalPortal_(other.illegalPortal_),
        relativeToPortal_(other.relativeToPortal_),
        timeDirection_(timeDirection)
{
}

bool Box::operator==(Box const& other) const
{
    return (x_ == other.x_)
        && (y_ == other.y_)
        && (xspeed_ == other.xspeed_)
        && (yspeed_ == other.yspeed_)
        && (size_ == other.size_)
        && (illegalPortal_ == other.illegalPortal_)
        && (relativeToPortal_ == other.relativeToPortal_)
        && (timeDirection_ == other.timeDirection_);
}

bool Box::operator<(const Box& other) const
{
    if (x_ == other.x_) {
        if (y_ == other.y_) {
            if (xspeed_ == other.xspeed_) {
                if (yspeed_ == other.yspeed_) {
                    if (size_ == other.size_) {
                        if (illegalPortal_ == other.illegalPortal_) {
                            if (relativeToPortal_ == other.relativeToPortal_) {
                                if (timeDirection_ == other.timeDirection_){
										return false;
                                }
                                return timeDirection_ < other.timeDirection_;
                            }
                            return relativeToPortal_ < other.relativeToPortal_;
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
}//namespace hg
