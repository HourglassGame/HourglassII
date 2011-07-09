#ifndef HG_ARRIVAL_LOCATION_H
#define HG_ARRIVAL_LOCATION_H
#include "TimeDirection.h"
namespace hg {
class ArrivalLocation {
public:
    ArrivalLocation(
        int x,
        int y,
        int xSpeed,
        int ySpeed,
        TimeDirection timeDirection) :
            x_(x), y_(y),
            xSpeed_(xSpeed), ySpeed_(ySpeed),
            timeDirection_(timeDirection) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xSpeed_; }
    int getYspeed() const { return ySpeed_; }
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }
        
private:
    int x_;
    int y_;
    int xSpeed_;
    int ySpeed_;
    TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_ARRIVAL_LOCATION_H
