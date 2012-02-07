#ifndef HG_RECTANGLE_GLITZ_H
#define HG_RECTANGLE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/cast.hpp>
namespace hg {
class RectangleGlitz: public GlitzImplementation {
public:
    RectangleGlitz(
        int x, int y,
        int width, int height,
        unsigned colour) :
            x_(x), y_(y),
            width_(width), height_(height),
            colour_(colour) {}
    virtual void display(Canvas& canvas) const {
        canvas.drawRect(x_/100., y_/100., width_/100., height_/100., colour_);
    }
    virtual std::size_t clone_size() const {
        return sizeof(*this);
    }
    virtual RectangleGlitz* perform_clone(void* memory) const {
        return new (memory) RectangleGlitz(*this);
    }
    virtual bool operator<(GlitzImplementation const& right) const {
        RectangleGlitz const& actual_right(*boost::polymorphic_downcast<RectangleGlitz const*>(&right));
        if (x_ == actual_right.x_) {
            if (y_ == actual_right.y_) {
                if (width_ == actual_right.width_) {
                    if (height_ == actual_right.height_) {
                        return colour_ < actual_right.colour_;
                    }
                    return height_ < actual_right.height_;
                }
                return width_ < actual_right.width_;
            }
            return y_ < actual_right.y_;
        }
        return x_ < actual_right.x_;
    }
    virtual bool operator==(GlitzImplementation const& o) const {
        RectangleGlitz const& actual_other(*boost::polymorphic_downcast<RectangleGlitz const*>(&o));
        return x_ == actual_other.x_
            && y_ == actual_other.y_
            && width_ == actual_other.width_
            && height_ == actual_other.height_
            && colour_ == actual_other.colour_;
    }
private:
    virtual int order_ranking() const {
        return 0;
    }
    int x_;
    int y_;
    int width_;
    int height_;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour_;
};
}
#endif //HG_RECTANGLE_GLITZ_H
