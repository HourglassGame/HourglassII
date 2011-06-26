#ifndef HG_GUY_H
#define HG_GUY_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include <cstdlib>
#include "SortWeakerThanEquality_fwd.h"
#include "ConstPtr_of_fwd.h"
namespace hg {
class Guy : boost::totally_ordered<Guy>
{
public:
    Guy(int x, int y,
        int xspeed, int yspeed,
        int width, int height,
        
        int illegalPortal_,
        int relativeToPortal,
        bool supported,
        int supportedSpeed,
    	
        bool facing,
    	
        bool boxCarrying,
    	int boxCarrySize,
    	TimeDirection boxCarryDirection,
    	int boxPauseLevel,
        
        TimeDirection timeDirection,
    	int pauseLevel,
        std::size_t index);

    
    struct increment_pause_level_tag {};
    Guy(const Guy& other, increment_pause_level_tag);
    Guy(const Guy& other, TimeDirection nTimeDirection, int nPauseLevel);
    
    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    
    int getIllegalPortal()    const { return illegalPortal_; }
    int getRelativeToPortal() const { return relativeToPortal_; }
    bool getSupported()       const { return supported_; }
    int getSupportedSpeed()   const { return supportedSpeed_; }
    
    int getFacing()        const { return facing_;}

    bool getBoxCarrying()  const { return boxCarrying_; }
    int getBoxCarrySize()  const { return boxCarrySize_; }
    TimeDirection 
        getBoxCarryDirection() const { return boxCarryDirection_; }
    int getBoxPauseLevel() const { return boxPauseLevel_; }

    TimeDirection
        getTimeDirection() const { return timeDirection_; }
    int getPauseLevel()    const { return pauseLevel_; }
    std::size_t getIndex() const { return index_; }

    bool operator==(const Guy& other) const;
    bool operator<(const Guy& second) const;
    
private:
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int width_;
    int height_;

    int illegalPortal_;
    int relativeToPortal_;
    bool supported_;
    int supportedSpeed_;

    bool facing_; // <- false, -> true

    bool boxCarrying_;
    int boxCarrySize_;
    TimeDirection boxCarryDirection_;
    int boxPauseLevel_;

    TimeDirection timeDirection_;
    int pauseLevel_;
    std::size_t index_;
};

class GuyConstPtr : boost::totally_ordered<GuyConstPtr>
{
public:
    GuyConstPtr(Guy const& guy) : guy_(&guy) {}
    typedef Guy base_type;
    Guy const& get() const   { return *guy_; }
    int getX()      const { return guy_->getX(); }
    int getY()      const { return guy_->getY(); }
    int getXspeed() const { return guy_->getXspeed(); }
    int getYspeed() const { return guy_->getYspeed(); }
    int getWidth()  const { return guy_->getWidth(); }
    int getHeight() const { return guy_->getHeight(); }
    
    int getIllegalPortal()    const { return guy_->getIllegalPortal(); }
    int getRelativeToPortal() const { return guy_->getRelativeToPortal(); }
    bool getSupported()       const { return guy_->getSupported(); }
    int getSupportedSpeed()   const { return guy_->getSupportedSpeed(); }
    
    int getFacing()        const { return guy_->getFacing();}

    bool getBoxCarrying()  const { return guy_->getBoxCarrying(); }
    int getBoxCarrySize()  const { return guy_->getBoxCarrySize(); }
    TimeDirection 
        getBoxCarryDirection() const { return guy_->getBoxCarryDirection(); }
    int getBoxPauseLevel() const { return guy_->getBoxPauseLevel(); }

    TimeDirection
        getTimeDirection() const { return guy_->getTimeDirection(); }
    int getPauseLevel()    const { return guy_->getPauseLevel(); }
    std::size_t getIndex() const { return guy_->getIndex(); }

    bool operator==(GuyConstPtr const& other) const { return *guy_ == *other.guy_; }
    bool operator<(GuyConstPtr const& other) const { return *guy_ < *other.guy_; }
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
