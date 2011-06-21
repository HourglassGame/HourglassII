#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
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
    Box(const Box& other, TimeDirection timeDirection, int pauseLevel);

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
    bool operator<(const Box& second) const;

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
}
#endif //HG_BOX_H
