#include "Box.h"

namespace hg {
Box::Box(int x, int y, int xspeed, int yspeed, int size, int illegalPortal, int relativeToPortal, TimeDirection timeDirection, int pauseLevel):
        x_(x),
        y_(y),
        xspeed_(xspeed),
        yspeed_(yspeed),
        size_(size),
        illegalPortal_(illegalPortal),
        relativeToPortal_(relativeToPortal),
        timeDirection_(timeDirection),
        pauseLevel_(pauseLevel)
{
}

Box::Box(const Box& other, hg::TimeDirection timeDirection, int pauseLevel) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        size_(other.size_),
        illegalPortal_(other.illegalPortal_),
        relativeToPortal_(other.relativeToPortal_),
        timeDirection_(timeDirection),
        pauseLevel_(pauseLevel)
{
}

Box::Box(const Box& other) :
        x_(other.x_),
        y_(other.y_),
        xspeed_(other.xspeed_),
        yspeed_(other.yspeed_),
        size_(other.size_),
        illegalPortal_(other.illegalPortal_),
        relativeToPortal_(other.relativeToPortal_),
        timeDirection_(other.timeDirection_),
        pauseLevel_(other.pauseLevel_)
{
}

Box& Box::operator=(const Box& other)
{
    x_ = other.x_;
    y_ = other.y_;
    xspeed_ = other.xspeed_;
    yspeed_ = other.yspeed_;
    size_ = other.size_;
    illegalPortal_ = other.illegalPortal_;
    relativeToPortal_ = other.relativeToPortal_;
    timeDirection_ = other.timeDirection_;
    pauseLevel_ = other.pauseLevel_;
    return *this;
}

bool Box::operator==(const Box& other) const
{
    return (x_ == other.x_)
           && (y_ == other.y_)
           && (xspeed_ == other.xspeed_)
           && (yspeed_ == other.yspeed_)
           && (size_ == other.size_)
           && (illegalPortal_ == other.illegalPortal_)
           && (relativeToPortal_ == other.relativeToPortal_)
           && (timeDirection_ == other.timeDirection_)
           && (pauseLevel_ == other.pauseLevel_);
}

bool Box::operator<(const Box& other) const
{
    if (x_ == other.x_)
    {
        if (y_ == other.y_)
        {
            if (xspeed_ == other.xspeed_)
            {
                if (yspeed_ == other.yspeed_)
                {
                    if (timeDirection_ == other.timeDirection_)
                    {
                        if (size_ == other.size_)
                        {
                            if (pauseLevel_ == other.pauseLevel_)
                            {
                            	if (illegalPortal_ == other.illegalPortal_)
								{
                            		if (relativeToPortal_ == other.relativeToPortal_)
									{
										return false;
									}
									else
									{
										return relativeToPortal_ < other.relativeToPortal_;
									}
								}
								else
								{
									return illegalPortal_ < other.illegalPortal_;
								}
                            }
                            else
                            {
                                return pauseLevel_ < other.pauseLevel_;
                            }
                        }
                        else
                        {
                            return size_ < other.size_;
                        }
                    }
                    else
                    {
                        return timeDirection_ < other.timeDirection_;
                    }
                }
                else
                {
                    return yspeed_ < other.yspeed_;
                }
            }
            else
            {
                return xspeed_ < other.xspeed_;
            }
        }
        else
        {
            return y_ < other.y_;
        }
    }
    else
    {
        return x_ < other.x_;
    }
}
}//namespace hg
