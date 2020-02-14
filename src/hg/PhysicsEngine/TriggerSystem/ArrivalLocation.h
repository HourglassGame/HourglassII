#ifndef HG_ARRIVAL_LOCATION_H
#define HG_ARRIVAL_LOCATION_H
#include "hg/TimeEngine/ArrivalDepartures/TimeDirection.h"
#include <ostream>
#include <tuple>
namespace hg {
class ArrivalLocation;
std::ostream &operator<<(std::ostream &os, ArrivalLocation const &toPrint);
class ArrivalLocation final {
    auto comparison_tuple() const noexcept
    {
        return std::tie(
            x_,
            y_,
            xspeed_,
            yspeed_,
            timeDirection_
        );
    }
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

    int getX()      const noexcept { return x_; }
    int getY()      const noexcept { return y_; }
    int getXspeed() const noexcept { return xspeed_; }
    int getYspeed() const noexcept { return yspeed_; }
    TimeDirection 
        getTimeDirection() const noexcept { return timeDirection_; }

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
    bool operator==(ArrivalLocation const &o) const noexcept {
        return comparison_tuple() == o.comparison_tuple();
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
