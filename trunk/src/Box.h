#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include <boost/tuple/tuple.hpp>
#include "ConstPtr_of_fwd.h"
#include <ostream>
namespace hg {
class Box;
std::ostream &operator<<(std::ostream &str, Box const &b);
class Box : boost::totally_ordered<Box>
{
public:
    Box(int x, int y,
        int xspeed, int yspeed,
        int size,
        int illegalPortal,
        int arrivalBasis,
        TimeDirection timeDirection);
    Box(Box const &o, TimeDirection timeDirection);

    int getX()      const { return x; }
    int getY()      const { return y; }
    int getXspeed() const { return xspeed; }
    int getYspeed() const { return yspeed; }
    int getSize()   const { return size; }
    int getWidth()  const { return size; }
    int getHeight() const { return size; }
    int getIllegalPortal()    const { return illegalPortal; }
	int getArrivalBasis() const { return arrivalBasis; }
    TimeDirection 
        getTimeDirection()    const { return timeDirection; }

    bool operator==(Box const &o) const;
    bool operator<(Box const &o) const;

private:
    int x;
    int y;
    int xspeed;
    int yspeed;
    int size;

    int illegalPortal;
    int arrivalBasis;

    TimeDirection timeDirection;
    
    #define HG_EQ_TIE_DEF boost::tie(\
                x, y, xspeed, yspeed, size,\
                illegalPortal, arrivalBasis,\
                timeDirection)
    
    auto as_tie() const -> decltype(HG_EQ_TIE_DEF)
    {
        return HG_EQ_TIE_DEF;
    }
    #undef HG_EQ_TIE_DEF
    //For debugging
    friend std::ostream &operator<<(std::ostream &str, Box const &b);
};

class BoxConstPtr : boost::totally_ordered<BoxConstPtr>
{
public:
    BoxConstPtr(Box const &box) : box_(&box) {}
    typedef Box base_type;
    Box const &get() const  { return *box_; }
    
    int getX()      const { return box_->getX(); }
    int getY()      const { return box_->getY(); }
    int getXspeed() const { return box_->getXspeed(); }
    int getYspeed() const { return box_->getYspeed(); }
    int getSize()   const { return box_->getSize(); }
    int getWidth()  const { return box_->getWidth(); }
    int getHeight() const { return box_->getHeight(); }
    int getIllegalPortal()const { return box_->getIllegalPortal(); }
	int getArrivalBasis() const { return box_->getArrivalBasis(); }
    TimeDirection 
        getTimeDirection()const { return box_->getTimeDirection(); }

    bool operator==(BoxConstPtr const &o) const { return *box_ == *o.box_; }
    bool operator< (BoxConstPtr const &o) const { return *box_ <  *o.box_; }

private:
    Box const *box_;
};
template<>
struct ConstPtr_of<Box> {
    typedef BoxConstPtr type;
};
}
#endif //HG_BOX_H
