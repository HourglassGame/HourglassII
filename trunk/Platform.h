#ifndef HG_PLATFORM_H
#define HG_PLATFORM_H
#include "TimeDirection.h"
#include "SortWeakerThanEquality_fwd.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
#include <cstddef>
namespace hg
{
class Platform : boost::totally_ordered<Platform>
{
public:
    Platform(
        int x, int y,
        int xspeed, int yspeed,
        int width, int height,
        std::size_t index,
        TimeDirection timeDirection,
        int pauseLevel);
    Platform(const Platform& other, TimeDirection timeDirection, int pauseLevel);

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    std::size_t getIndex() const { return index_; }
    TimeDirection 
        getTimeDirection() const { return timeDirection_; }
    int getPauseLevel()    const { return pauseLevel_; }

    bool operator==(const Platform& other) const;
    bool operator<(const Platform& other) const;

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
};

class PlatformConstPtr : boost::totally_ordered<PlatformConstPtr>
{
public:
    typedef Platform base_type;
    PlatformConstPtr(Platform const& platform) : platform_(&platform) {}
    Platform const& get() const { return *platform_; }
    
    int getX()      const { return platform_->getX(); }
    int getY()      const { return platform_->getY(); }
    int getXspeed() const { return platform_->getXspeed(); }
    int getYspeed() const { return platform_->getYspeed(); }
    int getWidth()  const { return platform_->getWidth(); }
    int getHeight() const { return platform_->getHeight(); }
    std::size_t getIndex() const { return platform_->getIndex(); }
    TimeDirection 
        getTimeDirection() const { return platform_->getTimeDirection(); }
    int getPauseLevel()    const { return platform_->getPauseLevel(); }

    bool operator==(const PlatformConstPtr& other) const { return *platform_ == *other.platform_; }
    bool operator<(const PlatformConstPtr& other) const { return *platform_ < *other.platform_; }

private:
   Platform const* platform_;
};

template<>
struct ConstPtr_of<Platform> {
    typedef PlatformConstPtr type;
};

template<>
struct sort_weaker_than_equality<Platform>
{
    static const bool value = true;
};

template<>
struct sort_weaker_than_equality<PlatformConstPtr>
{
    static const bool value = true;
};

}
#endif //HG_PLATFORM_H
