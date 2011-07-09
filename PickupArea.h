#ifndef HG_PICKUP_AREA_H
#define HG_PICKUP_AREA_H
#include "TimeDirection.h"
namespace hg {
enum PickupType {
    kTimeJump
};
class PickupArea {
public:
    PickupArea(
        int x,
        int y,
        int width,
        int height,
        int xSpeed,
        int ySpeed,
        PickupType type,
        TimeDirection timeDirection) :
            x_(x), y_(y),
            width_(width), height_(height),
            xSpeed_(xSpeed), ySpeed_(ySpeed),
            type_(type),
            timeDirection_(timeDirection) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    int getXspeed() const { return xSpeed_; }
    int getYspeed() const { return ySpeed_; }
    PickupType getType() const { return type_; }
    
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }
        
private:
    int x_;
    int y_;
    int width_;
    int height_;
    int xSpeed_;
    int ySpeed_;
    PickupType type_;
    TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_PICKUP_AREA_H
