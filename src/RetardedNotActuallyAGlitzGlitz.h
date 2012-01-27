#ifndef HG_RETARDED_NOT_ACTUALLY_A_GLITZ_GLITZ_H
#define HG_RETARDED_NOT_ACTUALLY_A_GLITZ_GLITZ_H
#include "TimeDirection.h"
namespace hg {
class RetardedNotActuallyAGlitzGlitz {
public:
    RetardedNotActuallyAGlitzGlitz(
        int x, int y,
        int width, int height,
        int xSpeed, int ySpeed,
        unsigned forwardsColour, unsigned reverseColour,
        TimeDirection timeDirection) :
            x_(x), y_(y),
            width_(width), height_(height),
            xSpeed_(xSpeed), ySpeed_(ySpeed),
            forwardsColour_(forwardsColour), reverseColour_(reverseColour),
            timeDirection_(timeDirection) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    int getXspeed() const { return xSpeed_; }
    int getYspeed() const { return ySpeed_; }
    unsigned getForwardsColour() const { return forwardsColour_; }
    unsigned getReverseColour() const { return reverseColour_; }
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }
        
private:
    int x_;
    int y_;
    int width_;
    int height_;
    int xSpeed_;
    int ySpeed_;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned forwardsColour_;
    unsigned reverseColour_;
    TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_RETARDED_NOT_ACTUALLY_A_GLITZ_GLITZ_H
