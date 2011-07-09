#ifndef HG_KILLER_AREA_H
#define HG_KILLER_AREA_H
#include "TimeDirection.h"
namespace hg {
class KillerArea {
public:
    KillerArea(
        int x,
        int y,
        int width,
        int height,
        int xSpeed,
        int ySpeed,
        TimeDirection timeDirection) :
            x_(x), y_(y),
            width_(width), height_(height),
            xSpeed_(xSpeed), ySpeed_(ySpeed),
            timeDirection_(timeDirection) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    int getXspeed() const { return xSpeed_; }
    int getYspeed() const { return ySpeed_; }
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }
        
private:
    int x_;
    int y_;
    int width_;
    int height_;
    int xSpeed_;
    int ySpeed_;
    TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_KILLER_AREA_H
