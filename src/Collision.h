#ifndef HG_COLLISION_H
#define HG_COLLISION_H
#include "TimeDirection.h"
#include <ostream>
namespace hg
{
class Collision
{
public:
    Collision(
        int x, int y,
        int xspeed, int yspeed,
        int width, int height,
        TimeDirection timeDirection) :
            x_(x), y_(y),
            xspeed_(xspeed), yspeed_(yspeed),
            width_(width), height_(height),
            timeDirection_(timeDirection) { }
    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }
    friend std::ostream &operator<<(std::ostream &os, Collision const &toPrint)
    {
        #define HG_COLLISION_PRINT(obj) os << #obj << ": " << toPrint.obj << std::endl
        HG_COLLISION_PRINT(x_);
        HG_COLLISION_PRINT(y_);
        HG_COLLISION_PRINT(xspeed_);
        HG_COLLISION_PRINT(yspeed_);
        HG_COLLISION_PRINT(width_);
        HG_COLLISION_PRINT(height_);
        HG_COLLISION_PRINT(timeDirection_);
        #undef HG_COLLISION_PRINT
        return os;
    }
private:
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int width_;
    int height_;
    TimeDirection timeDirection_;
};

}
#endif //HG_COLLISION_H
