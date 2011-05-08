#ifndef HG_PLATFORM_H
#define HG_PLATFORM_H
#include "TimeDirection.h"
#include "SortedByIndex_fwd.h"
#include <cstddef>
namespace hg
{
class Platform
{
public:
    Platform(int x, int y, int xspeed, int yspeed, int width, int height, std::size_t index, TimeDirection timeDirection, int pauseLevel);
    Platform(const Platform& other, TimeDirection timeDirection, int pauseLevel);
    Platform(const Platform& other);
    Platform& operator=(const Platform& other);

    int getX() const {
        return x_;
    }
    int getY() const {
        return y_;
    }
    int getXspeed() const {
        return xspeed_;
    }
    int getYspeed() const {
        return yspeed_;
    }
    int getWidth() const {
        return width_;
    }
    int getHeight() const {
        return height_;
    }
    std::size_t getIndex() const {
        return index_;
    }
    TimeDirection getTimeDirection() const {
        return timeDirection_;
    }
    int getPauseLevel() const {
        return pauseLevel_;
    }

    bool operator==(const Platform& other) const;
    bool operator!=(const Platform& other) const;

    bool operator<(const Platform& second) const;

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
template<>
class sorted_by_index<Platform>
{
    static const bool value = true;
};
}
#endif //HG_PLATFORM_H
