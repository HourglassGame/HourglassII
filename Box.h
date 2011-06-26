#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
namespace hg {
class Box : boost::totally_ordered<Box>
{
public:
    Box(int x, int y,
        int xspeed, int yspeed,
        int size,
        int illegalPortal,
        int relativeToPortal,
        TimeDirection timeDirection,
        int pauseLevel);
    Box(Box const& other, TimeDirection timeDirection, int pauseLevel);

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    int getSize()   const { return size_; }
    int getWidth()  const { return size_; }
    int getHeight() const { return size_; }
    int getIllegalPortal()    const { return illegalPortal_; }
	int getRelativeToPortal() const { return relativeToPortal_; }
    TimeDirection 
        getTimeDirection()    const { return timeDirection_; }
    int getPauseLevel()       const { return pauseLevel_; }

    bool operator==(const Box& other) const;
    bool operator<(const Box& other) const;

private:

    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int size_;

    int illegalPortal_;
    int relativeToPortal_;

    TimeDirection timeDirection_;
    int pauseLevel_;
};

class BoxConstPtr : boost::totally_ordered<BoxConstPtr>
{
public:
    BoxConstPtr(Box const& box) : box_(&box) {}
    typedef Box base_type;
    Box const& get() const  { return *box_; }
    
    int getX()      const { return box_->getX(); }
    int getY()      const { return box_->getY(); }
    int getXspeed() const { return box_->getXspeed(); }
    int getYspeed() const { return box_->getYspeed(); }
    int getSize()   const { return box_->getSize(); }
    int getWidth()  const { return box_->getWidth(); }
    int getHeight() const { return box_->getHeight(); }
    int getIllegalPortal()    const { return box_->getIllegalPortal(); }
	int getRelativeToPortal() const { return box_->getRelativeToPortal(); }
    TimeDirection 
        getTimeDirection()    const { return box_->getTimeDirection(); }
    int getPauseLevel()       const { return box_->getPauseLevel(); }

    bool operator==(const BoxConstPtr& other) const { return *box_ == *other.box_; }
    bool operator<(const BoxConstPtr& other) const { return *box_ < *other.box_; }

private:
    Box const* box_;
};
template<>
struct ConstPtr_of<Box> {
    typedef BoxConstPtr type;
};
}
#endif //HG_BOX_H
