#ifndef HG_BUTTON_H
#define HG_BUTTON_H
#include "TimeDirection.h"
#include "SortedByIndex_fwd.h"
#include <cstddef>
namespace hg
{
class Button
{

public:

    Button(int x, int y, int xspeed, int yspeed, int width, int height, std::size_t index, bool state, TimeDirection timeDirection, int pauseLevel);
    Button(int x, int y, int xspeed, int yspeed, const Button& other);
    Button(const Button& other, TimeDirection timeDirection, int pauseLevel);
    Button(const Button& other);
    Button& operator=(const Button& other);

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
    bool getState() const {
        return state_;
    }
    TimeDirection getTimeDirection() const {
        return timeDirection_;
    }
    int getPauseLevel() const {
        return pauseLevel_;
    }

    bool operator==(const Button& other) const;
    bool operator!=(const Button& other) const;

    bool operator<(const Button& second) const;

private:
    int x_;
    int y_;
    int xspeed_;
    int yspeed_;
    int width_;
    int height_;
    std::size_t index_;
    bool state_;
    TimeDirection timeDirection_;
    int pauseLevel_;
};
template<>
struct sorted_by_index<Button>
{
    static const bool value = true;
};
}
#endif //HG_BUTTON_H
