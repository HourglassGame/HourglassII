#ifndef HG_PORTAL_AREA_H
#define HG_PORTAL_AREA_H
#include "TimeDirection.h"
#include <ostream>
namespace hg {
class PortalArea;
std::ostream &operator<<(std::ostream &os, PortalArea const &toPrint);
class PortalArea {
public:
    PortalArea(
        int index,
        int x,
        int y,
        int width,
        int height,
        int xspeed,
        int yspeed,
        int collisionOverlap,
        TimeDirection timeDirection,
        int destinationIndex,
        int xDestination,
        int yDestination,
        bool relativeTime,
        int timeDestination,
		bool relativeDirection,
        TimeDirection destinationDirection,
        int illegalDestination,
        bool fallable,
        bool winner
        ) :
            index_(index),
            x_(x),
            y_(y),
            width_(width),
            height_(height),
            xspeed_(xspeed),
            yspeed_(yspeed),
            collisionOverlap_(collisionOverlap),
            timeDirection_(timeDirection),
            destinationIndex_(destinationIndex),
            xDestination_(xDestination),
            yDestination_(yDestination),
            relativeTime_(relativeTime),
            timeDestination_(timeDestination),
			relativeDirection_(relativeDirection),
            destinationDirection_(destinationDirection),
            illegalDestination_(illegalDestination),
            fallable_(fallable),
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
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    
    int getCollisionOverlap() const { return collisionOverlap_; }
    
    TimeDirection
        getTimeDirection() const { return timeDirection_; }
    
    int getDestinationIndex() const { return destinationIndex_; }
    int getXdestination() const { return xDestination_; }
    int getYdestination() const { return yDestination_; }
    bool getRelativeTime() const { return relativeTime_; }
    int getTimeDestination() const { return timeDestination_; }
	bool getRelativeDirection() const { return relativeDirection_; }
    TimeDirection getDestinationDirection() const { return destinationDirection_; }
	
    int getIllegalDestination() const { return illegalDestination_; }
    bool getFallable() const { return fallable_; }
    bool getWinner() const { return winner_; }
    
    friend std::ostream &operator<<(std::ostream &os, PortalArea const &toPrint)
    {
        os << '{';
#define HG_PORTAL_AREA_PRINT(field) os << #field << "=" << toPrint.field
        HG_PORTAL_AREA_PRINT(index_) << ',';
        HG_PORTAL_AREA_PRINT(x_) << ',';
        HG_PORTAL_AREA_PRINT(y_) << ',';
        HG_PORTAL_AREA_PRINT(width_) << ',';
        HG_PORTAL_AREA_PRINT(height_) << ',';
        HG_PORTAL_AREA_PRINT(xspeed_) << ',';
        HG_PORTAL_AREA_PRINT(yspeed_) << ',';
        HG_PORTAL_AREA_PRINT(collisionOverlap_) << ',';
        HG_PORTAL_AREA_PRINT(timeDirection_) << ',';
        HG_PORTAL_AREA_PRINT(destinationIndex_) << ',';
        HG_PORTAL_AREA_PRINT(xDestination_) << ',';
        HG_PORTAL_AREA_PRINT(yDestination_) << ',';
        HG_PORTAL_AREA_PRINT(relativeTime_) << ',';
        HG_PORTAL_AREA_PRINT(timeDestination_) << ',';
		HG_PORTAL_AREA_PRINT(relativeDirection_) << ',';
        HG_PORTAL_AREA_PRINT(destinationDirection_) << ',';
        HG_PORTAL_AREA_PRINT(illegalDestination_) << ',';
        HG_PORTAL_AREA_PRINT(fallable_) << ',';
        HG_PORTAL_AREA_PRINT(winner_);
#undef HG_PORTAL_AREA_PRINT
        os << '}';
        return os;
    }
    
private:
    int index_;
    int x_;
    int y_;
    int width_;
    int height_;
    int xspeed_;
    int yspeed_;
    int collisionOverlap_;
    TimeDirection timeDirection_;
    int destinationIndex_;
    int xDestination_;
    int yDestination_;
    bool relativeTime_;
    int timeDestination_;
	bool relativeDirection_;
    TimeDirection destinationDirection_;
    int illegalDestination_;
    bool fallable_;
    bool winner_;
};
}//namespace hg
#endif //HG_PORTAL_AREA_H
