#ifndef HG_BUTTON_H
#define HG_BUTTON_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "SortWeakerThanEquality_fwd.h"
#include "ConstPtr_of_fwd.h"
#include <cstddef>
namespace hg
{
class Button : boost::totally_ordered<Button>
{
public:
    Button(
        int x, int y,
        int xspeed, int yspeed,
        int width, int height,
        std::size_t index,
        bool state,
        TimeDirection timeDirection);
    Button(int x, int y, int xspeed, int yspeed, const Button& other);
    Button(const Button& other, TimeDirection timeDirection);

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getXspeed() const { return xspeed_; }
    int getYspeed() const { return yspeed_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }
    std::size_t getIndex() const { return index_; }
    bool getState() const { return state_; }
    TimeDirection
        getTimeDirection() const { return timeDirection_; }

    bool operator==(Button const& other) const;
    bool operator<(Button const& other) const;

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
};
class ButtonConstPtr : boost::totally_ordered<ButtonConstPtr>
{
public:
    ButtonConstPtr(Button const& button) : button_(&button) {}
    typedef Button base_type;
    Button const& get() const  { return *button_; }
    int getX()      const { return button_->getX(); }
    int getY()      const { return button_->getY(); }
    int getXspeed() const { return button_->getXspeed(); }
    int getYspeed() const { return button_->getYspeed(); }
    int getWidth()  const { return button_->getWidth(); }
    int getHeight() const { return button_->getHeight(); }
    std::size_t getIndex() const { return button_->getIndex(); }
    bool getState() const { return button_->getState(); }
    TimeDirection
        getTimeDirection() const { return button_->getTimeDirection(); }

    bool operator==(ButtonConstPtr const& other) const { return *button_ == *other.button_; }
    bool operator<(ButtonConstPtr const& other) const { return *button_ < *other.button_; }

private:
    Button const* button_;
};
template<>
struct ConstPtr_of<Button> {
    typedef ButtonConstPtr type;
};
template<>
struct sort_weaker_than_equality<Button>
{
    static const bool value = true;
};
template<>
struct sort_weaker_than_equality<ButtonConstPtr>
{
    static const bool value = true;
};
}
#endif //HG_BUTTON_H
