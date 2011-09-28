#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
#include <ostream>
namespace hg {
class Box : boost::totally_ordered<Box>
{
public:
    Box(int x, int y,
        int xspeed, int yspeed,
        int size,
        int illegalPortal,
        int arrivalBasis,
        TimeDirection timeDirection);
    Box(Box const& other, TimeDirection timeDirection);

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    int getSize()   const { return size_; }
    int getWidth()  const { return size_; }
    int getHeight() const { return size_; }
    int getIllegalPortal()    const { return illegalPortal_; }
	int getArrivalBasis() const { return arrivalBasis_; }
    TimeDirection 
        getTimeDirection()    const { return timeDirection_; }

    bool operator==(const Box& other) const;
    bool operator<(const Box& other) const;

private:

    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int size_;

    int illegalPortal_;
    int arrivalBasis_;

    TimeDirection timeDirection_;
    //For debugging
    friend std::ostream& operator<<(std::ostream& str, Box const& b);
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
	int getArrivalBasis() const { return box_->getArrivalBasis(); }
    TimeDirection 
        getTimeDirection()    const { return box_->getTimeDirection(); }

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
