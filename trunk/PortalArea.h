#ifndef HG_PORTAL_AREA_H
#define HG_PORTAL_AREA_H
#include "TimeDirection.h"
namespace hg {
class PortalArea {
public:
    PortalArea(
        int index,
        int x,
        int y,
        int width,
        int height,
        int xSpeed,
        int ySpeed,
        TimeDirection timeDirection,
        int destinationIndex,
        int xDestination,
        int yDestination,
        bool relativeTime,
        int timeDestination,
        int illegalDestination,
        bool fallable,
        int collisionOverlap,
        bool winner
        ) :
            index_(index),
            x_(x),
            y_(y),
            width_(width),
            height_(height),
            xSpeed_(xSpeed),
            ySpeed_(ySpeed),
            timeDirection_(timeDirection),
            destinationIndex_(destinationIndex),
            xDestination_(xDestination),
            yDestination_(yDestination),
            relativeTime_(relativeTime),
            timeDestination_(timeDestination),
            illegalDestination_(illegalDestination),
            fallable_(fallable),
            collisionOverlap_(collisionOverlap),
            winner_(winner)
            {
            }
    //Index is used for identifying illegal-portals
    //Maybe we should add a simple way for
    //triggers to attach information to guys.
    //This would allow `shouldPort` to manage
    //illegal-portals.
    int getIndex()    const { return index_; }
    
    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    int getXspeed() const { return xSpeed_; }
    int getYspeed() const { return ySpeed_; }
    
    TimeDirection
        getTimeDirection() const { return timeDirection_; }
    
    int getDestinationIndex() const { return destinationIndex_; }
    int getXdestination() const { return xDestination_; }
    int getYdestination() const { return yDestination_; }
    bool getRelativeTime() const { return relativeTime_; }
    int getTimeDestination() const { return timeDestination_; }
    int getIllegalDestination() const { return illegalDestination_; }
    bool getFallable() const { return fallable_; }
    int getCollisionOverlap() const { return collisionOverlap_; };
    bool getWinner() const { return winner_; }
private:
    int index_;
    int x_;
    int y_;
    int width_;
    int height_;
    int xSpeed_;
    int ySpeed_;
    TimeDirection timeDirection_;
    int destinationIndex_;
    int xDestination_;
    int yDestination_;
    bool relativeTime_;
    int timeDestination_;
    int illegalDestination_;
    bool fallable_;
    int collisionOverlap_;
    bool winner_;
    
};
}//namespace hg
#endif //HG_PORTAL_AREA_H
