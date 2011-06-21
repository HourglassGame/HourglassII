#ifndef HG_PORTAL_H
#define HG_PORTAL_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "SortWeakerThanEquality_fwd.h"
#include <cstddef>
namespace hg
{
class Portal : boost::totally_ordered<Portal>
{
public:
    Portal(
        int x, int y,
        int xspeed, int yspeed,
        int width, int height,
        std::size_t index,
        TimeDirection timeDirection,
        int pauseLevel,
        int charges,
        bool active,
        int xDestination, int yDestination,
        int destinationIndex, int timeDestination,
        bool relativeTime,
        int illegalDestination,
        bool fallable,
        bool winner);
    Portal(int x, int y, int xspeed, int yspeed, const Portal& other);
    Portal(const Portal& other, TimeDirection timeDirection, int pauseLevel);

    int getX()              const { return x_; }
    int getY()              const { return y_; }
    int getXspeed()         const { return xspeed_; }
    int getYspeed()         const { return yspeed_; }
    int getWidth()          const { return width_; }
    int getHeight()         const { return height_; }
    std::size_t getIndex()  const { return index_; }
    TimeDirection
        getTimeDirection()  const { return timeDirection_; }
    int getPauseLevel()     const { return pauseLevel_; }
    int getCharges()        const { return charges_; }
    bool getActive()        const { return active_; }
    int getXdestination()   const { return xDestination_; }
    int getYdestination()   const { return yDestination_; }
    int getDestinationIndex()const{ return destinationIndex_; }
    int getTimeDestination()const { return timeDestination_; }
    bool getRelativeTime()  const { return relativeTime_; }
    int getIllegalDestination()const{return illegalDestination_; }
    int getFallable()       const { return fallable_; }
    bool getWinner()        const { return winner_; }

    bool operator==(const Portal& other) const;
    bool operator<(const Portal& second) const;

private:
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int width_;
    int height_;
    std::size_t index_;
    TimeDirection timeDirection_;
    int pauseLevel_;
    int charges_;
    bool active_;
    int xDestination_;
    int yDestination_;
    int destinationIndex_;
    int timeDestination_;
    bool relativeTime_;
    int illegalDestination_;
    bool fallable_;
    bool winner_;
};
template<>
struct sort_weaker_than_equality<Portal>
{
    static const bool value = true;
};
}
#endif //HG_PLATFORM_H
