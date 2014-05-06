#ifndef HG_ARRIVAL_LOCATION_H
#define HG_ARRIVAL_LOCATION_H
#include "TimeDirection.h"
#include <ostream>
namespace hg {
class ArrivalLocation;
std::ostream &operator<<(std::ostream &os, ArrivalLocation const &toPrint);
class ArrivalLocation {
public:
    ArrivalLocation(
        int x,
        int y,
        int xspeed,
        int yspeed,
        TimeDirection timeDirection) :
            x_(x), y_(y),
            xspeed_(xspeed), yspeed_(yspeed),
            timeDirection_(timeDirection) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }

    friend std::ostream &operator<<(std::ostream &os, ArrivalLocation const &toPrint)
    {
        #define HG_ARRIVAL_LOCATION_PRINT(obj) os << #obj << ": " << toPrint.obj << std::endl
        HG_ARRIVAL_LOCATION_PRINT(x_);
        HG_ARRIVAL_LOCATION_PRINT(y_);
        HG_ARRIVAL_LOCATION_PRINT(xspeed_);
        HG_ARRIVAL_LOCATION_PRINT(yspeed_);
        HG_ARRIVAL_LOCATION_PRINT(timeDirection_);
        #undef HG_ARRIVAL_LOCATION_PRINT
        return os;
    }

private:
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_ARRIVAL_LOCATION_H
