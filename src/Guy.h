#ifndef HG_GUY_H
#define HG_GUY_H
#include "TimeDirection.h"
#include "Ability.h"
#include "FacingDirection.h"
#include <boost/operators.hpp>
#include "mt/std/map"
#include <cstdlib>
#include "SortWeakerThanEquality_fwd.h"
#include "ConstPtr_of_fwd.h"
namespace hg {
class Guy : boost::totally_ordered<Guy>
{
public:
    Guy(std::size_t index,
        int x, int y,
        int xspeed, int yspeed,
        int width, int height,
        int jumpSpeed,
        
        int illegalPortal,
        int arrivalBasis,
        bool supported,
        int supportedSpeed,
    	
        mt::std::map<Ability, int>::type const& pickups,
        FacingDirection::FacingDirection facing,
    	
        bool boxCarrying,
    	int boxCarrySize,
    	TimeDirection boxCarryDirection,
        
        TimeDirection timeDirection,
		bool timePaused);

    Guy(const Guy& o, TimeDirection nTimeDirection, bool nTimePaused);
    
    std::size_t getIndex() const { return index_; }
    int getX()         const { return x_; }
    int getY()         const { return y_; }
    int getXspeed()    const { return xspeed_; }
    int getYspeed()    const { return yspeed_; }
    int getWidth()     const { return width_; }
    int getHeight()    const { return height_; }
    int getJumpSpeed() const { return jumpSpeed_; }
    
    int getIllegalPortal()    const { return illegalPortal_; }
    int getArrivalBasis() const { return arrivalBasis_; }
    bool getSupported()       const { return supported_; }
    int getSupportedSpeed()   const { return supportedSpeed_; }
    
    mt::std::map<Ability, int>::type const& getPickups() const { return pickups_; }

    FacingDirection::FacingDirection getFacing()        const { return facing_; }

    bool getBoxCarrying()  const { return boxCarrying_; }
    int getBoxCarrySize()  const { return boxCarrySize_; }
    TimeDirection 
        getBoxCarryDirection() const { return boxCarryDirection_; }

    TimeDirection
        getTimeDirection() const { return timeDirection_; }
	bool getTimePaused()    const { return timePaused_; }


    bool operator==(const Guy& o) const;
    bool operator<(const Guy& second) const;
    
private:
    std::size_t index_;
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int width_;
    int height_;
    int jumpSpeed_;

    int illegalPortal_;
    int arrivalBasis_;
    bool supported_;
    int supportedSpeed_;

    mt::std::map<Ability, int>::type pickups_;
    FacingDirection::FacingDirection facing_; // <- false, -> true

    bool boxCarrying_;
    int boxCarrySize_;
    TimeDirection boxCarryDirection_;

    TimeDirection timeDirection_;
	bool timePaused_;
};

class GuyConstPtr : boost::totally_ordered<GuyConstPtr>
{
public:
    GuyConstPtr(Guy const& guy) : guy_(&guy) {}
    typedef Guy base_type;
    Guy const& get() const   { return *guy_; }
    
    std::size_t getIndex() const { return guy_->getIndex(); }
    int getX()         const { return guy_->getX(); }
    int getY()         const { return guy_->getY(); }
    int getXspeed()    const { return guy_->getXspeed(); }
    int getYspeed()    const { return guy_->getYspeed(); }
    int getWidth()     const { return guy_->getWidth(); }
    int getHeight()    const { return guy_->getHeight(); }
    int getJumpSpeed() const { return guy_->getJumpSpeed(); }
    
    int getIllegalPortal()    const { return guy_->getIllegalPortal(); }
    int getArrivalBasis() const { return guy_->getArrivalBasis(); }
    bool getSupported()       const { return guy_->getSupported(); }
    int getSupportedSpeed()   const { return guy_->getSupportedSpeed(); }
    
    mt::std::map<Ability, int>::type const& getPickups() const { return guy_->getPickups();}

    FacingDirection::FacingDirection getFacing()        const { return guy_->getFacing();}

    bool getBoxCarrying()  const { return guy_->getBoxCarrying(); }
    int getBoxCarrySize()  const { return guy_->getBoxCarrySize(); }
    TimeDirection 
        getBoxCarryDirection() const { return guy_->getBoxCarryDirection(); }

    TimeDirection
        getTimeDirection() const { return guy_->getTimeDirection(); }
	bool getTimePaused()    const { return guy_->getTimePaused(); }

    bool operator==(GuyConstPtr const& o) const { return *guy_ == *o.guy_; }
    bool operator<(GuyConstPtr const& o) const { return *guy_ < *o.guy_; }
private:
    Guy const* guy_;
};

template<>
struct ConstPtr_of<Guy> {
    typedef GuyConstPtr type;
};

template<>
struct sort_weaker_than_equality<Guy>
{
    static const bool value = true;
};

template<>
struct sort_weaker_than_equality<GuyConstPtr>
{
    static const bool value = true;
};
}
#endif //HG_GUY_H
