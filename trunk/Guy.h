#ifndef HG_GUY_H
#define HG_GUY_H
#include "TimeDirection.h"
#include <cstdlib>
#include "SortedByIndex.h"
namespace hg {
class Guy
{
public:
    Guy(int x,int y,int xspeed,int yspeed,int width,int height,int relativeToPortal,bool supported,bool boxCarrying,
        int boxCarrySize,TimeDirection boxCarryDirection,int boxPauseLevel,TimeDirection timeDirection,int pauseLevel,
        std::size_t index);

    Guy(const Guy& other, TimeDirection nTimeDirection, int nPauseLevel);
    Guy(const Guy& other);
    Guy& operator=(const Guy& other);

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

    int getRelativeToPortal() const {
        return relativeToPortal_;
    }
    int getSupported() const {
        return supported_;
    }

    bool getBoxCarrying() const {
        return boxCarrying_;
    }
    int getBoxCarrySize() const {
        return boxCarrySize_;
    }
    TimeDirection getBoxCarryDirection() const {
        return boxCarryDirection_;
    }
    int getBoxPauseLevel() const {
        return boxPauseLevel_;
    }

    TimeDirection getTimeDirection() const {
        return timeDirection_;
    }
    int getPauseLevel() const {
        return pauseLevel_;
    }
    std::size_t getIndex() const {
        return index_;
    }

    bool operator!=(const Guy& other) const;
    bool operator==(const Guy& other) const;

    bool operator<(const Guy& second) const;
private:
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int width_;
    int height_;

    int relativeToPortal_;
    bool supported_;

    bool boxCarrying_;
    int boxCarrySize_;
    TimeDirection boxCarryDirection_;
    int boxPauseLevel_;

    TimeDirection timeDirection_;
    int pauseLevel_;
    std::size_t index_;
};
template<>
struct sorted_by_index<Guy>
{
    static const bool value = true;
};
}
#endif //HG_GUY_H
