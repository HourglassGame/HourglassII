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
        
        mt::std::map<Ability, int> pickups,
        FacingDirection facing,
        
        bool boxCarrying,
        int boxCarrySize,
        TimeDirection boxCarryDirection,
        
        TimeDirection timeDirection,
        bool timePaused);
    
    std::size_t getIndex() const { return index; }
    int getX()         const { return x; }
    int getY()         const { return y; }
    int getXspeed()    const { return xspeed; }
    int getYspeed()    const { return yspeed; }
    int getWidth()     const { return width; }
    int getHeight()    const { return height; }
    int getJumpSpeed() const { return jumpSpeed; }
    
    int getIllegalPortal()    const { return illegalPortal; }
    int getArrivalBasis() const { return arrivalBasis; }
    bool getSupported()       const { return supported; }
    int getSupportedSpeed()   const { return supportedSpeed; }
    
    mt::std::map<Ability, int> const &getPickups() const { return pickups; }

    FacingDirection getFacing()        const { return facing; }

    bool getBoxCarrying()  const { return boxCarrying; }
    int getBoxCarrySize()  const { return boxCarrySize; }
    TimeDirection 
        getBoxCarryDirection() const { return boxCarryDirection; }

    TimeDirection
        getTimeDirection() const { return timeDirection; }
    bool getTimePaused()    const { return timePaused; }


    bool operator==(Guy const &o) const;
    bool operator<(Guy const &second) const;
    
private:
    std::size_t index;
    int x;
    int y;
    int xspeed;
    int yspeed;
    int width;
    int height;
    int jumpSpeed;

    int illegalPortal;
    int arrivalBasis;
    bool supported;
    int supportedSpeed;

    mt::std::map<Ability, int> pickups;
    FacingDirection facing; // <- 0, -> 1

    bool boxCarrying;
    int boxCarrySize;
    TimeDirection boxCarryDirection;

    TimeDirection timeDirection;
    bool timePaused;
    
    auto equality_tuple() const -> decltype(auto)
    {
        return std::tie(
            index, 
            x, y, xspeed, yspeed, width, height, jumpSpeed,
            illegalPortal, arrivalBasis, supported, supportedSpeed,
            pickups, facing,
            boxCarrying, boxCarrySize, boxCarryDirection,
            timeDirection, timePaused);
    }
};

class GuyConstPtr : boost::totally_ordered<GuyConstPtr>
{
public:
    GuyConstPtr(Guy const &guy) : guy_(&guy) {}
    typedef Guy base_type;
    Guy const &get() const   { return *guy_; }
    
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
    
    mt::std::map<Ability, int> const &getPickups() const { return guy_->getPickups();}

    FacingDirection getFacing()        const { return guy_->getFacing();}

    bool getBoxCarrying()  const { return guy_->getBoxCarrying(); }
    int getBoxCarrySize()  const { return guy_->getBoxCarrySize(); }
    TimeDirection 
        getBoxCarryDirection() const { return guy_->getBoxCarryDirection(); }

    TimeDirection
        getTimeDirection() const { return guy_->getTimeDirection(); }
    bool getTimePaused()    const { return guy_->getTimePaused(); }

    bool operator==(GuyConstPtr const &o) const { return *guy_ == *o.guy_; }
    bool operator<(GuyConstPtr const &o) const { return *guy_ < *o.guy_; }
private:
    Guy const *guy_;
};

template<>
struct ConstPtr_of<Guy> {
    typedef GuyConstPtr type;
};

template<>
struct sort_weaker_than_equality<Guy>
{
    static bool const value = true;
};

template<>
struct sort_weaker_than_equality<GuyConstPtr>
{
    static bool const value = true;
};
}
#endif //HG_GUY_H
