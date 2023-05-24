#ifndef HG_PORTAL_AREA_H
#define HG_PORTAL_AREA_H
#include "hg/TimeEngine/ArrivalDepartures/TimeDirection.h"
#include <ostream>
#include <tuple>
namespace hg {
class PortalArea;
std::ostream &operator<<(std::ostream &os, PortalArea const &toPrint);
class PortalArea final {
public:
	PortalArea(
		int index,
		int x,
		int y,
		int xaim,
		int yaim,
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
		bool guyOnly,
		bool isLaser,
		bool winner
		) :
			index_(index),
			x_(x),
			y_(y),
			xaim_(xaim),
			yaim_(yaim),
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
			guyOnly_(guyOnly),
			isLaser_(isLaser),
			winner_(winner)
	{
		//not strictly necessary, just providing normalised values where the values are not used
		if (winner_) {
			winner_ = true;
			relativeTime_ = false;
			timeDestination_ = 1;
			relativeDirection_ = false;
			destinationDirection_ = TimeDirection::FORWARDS;
		}
	}
	//Index is used for identifying illegal-portals
	//Maybe we should add a simple way for
	//triggers to attach information to guys.
	//This would allow `shouldPort` to manage
	//illegal-portals.
	int getIndex()    const { return index_; }
	
	int getX()      const { return x_; }
	int getY()      const { return y_; }
	int getXaim()      const { return xaim_; }
	int getYaim()      const { return yaim_; }
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
	bool getGuyOnly() const { return guyOnly_; }
	bool getIsLaser() const { return isLaser_; }
	bool getWinner() const { return winner_; }
	
	friend std::ostream &operator<<(std::ostream &os, PortalArea const &toPrint)
	{
		os << '{';
#define HG_PORTAL_AREA_PRINT(field) os << #field << "=" << toPrint.field
		HG_PORTAL_AREA_PRINT(index_) << ',';
		HG_PORTAL_AREA_PRINT(x_) << ',';
		HG_PORTAL_AREA_PRINT(y_) << ',';
		HG_PORTAL_AREA_PRINT(xaim_) << ',';
		HG_PORTAL_AREA_PRINT(yaim_) << ',';
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
		HG_PORTAL_AREA_PRINT(guyOnly_) << ',';
		HG_PORTAL_AREA_PRINT(isLaser_) << ',';
		HG_PORTAL_AREA_PRINT(winner_);
#undef HG_PORTAL_AREA_PRINT
		os << '}';
		return os;
	}
	bool operator==(PortalArea const& o) const = default;
private:
	//The Illegal-Portal system explained:
	//The motivation for Illegal-Portal is that we sometimes want fallable
	//portals where the guy/box will not immediately fall through a new portal when they arrive.
	//That is --- we sometimes want to make it so guys/boxes cannot immediately fall through portals
	//upon arriving, but must experience at least one frame of 'not intersecting with the portal' before
	//'intersecting with the portal' will once again trigger porting.
	
	//This behaviour is mediated by three properties:
	//Portal.index_: A positive integer identifier (not necessecarily unique) given to each portal
	//Portal.illegalDestination_: The "index_" of the portal that the arriving guy shouldn't fall through
	//(Guy/Box).illegalPortal: The "index_" of the portal that the guy/box mustn't fall through until it has 'walked off' the portal.
	//                         `-1` indicates that the guy/box may fall through any portal.

	//This behaviour is implemented by hg::PhysicsEngine (at time of writing: PhysicsEngineUtilities_def.h).
	int index_;
	int x_;
	int y_;
	int xaim_;
	int yaim_;
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
	bool guyOnly_;
	bool isLaser_;
	bool winner_;
};
}//namespace hg
#endif //HG_PORTAL_AREA_H
