#ifndef HG_GLITZ_H
#define HG_GLITZ_H
#include <boost/operators.hpp>
namespace hg {
class Glitz : boost::totally_ordered<Glitz> {
public:
    Glitz(
        int x, int y,
        int width, int height,
        unsigned colour) :
            x_(x), y_(y),
            width_(width), height_(height),
            colour_(colour) {}

    int getX()      const { return x_; }
    int getY()      const { return y_; }
    int getWidth()  const { return width_; }
    int getHeight() const { return height_; }

    unsigned getColour() const { return colour_; }
    
    //Glitz has to be ordered (for sorting arrivals),
    //but since it can be pretty much
    //arbitrary drawing or other SFX/GFX (this is not yet implemented),
    //this ordering will too have to be pretty arbitrary unfortunately.
    bool operator<(Glitz const& right) const {
        if (x_ == right.x_) {
            if (y_ == right.y_) {
                if (width_ == right.width_) {
                    if (height_ == right.height_) {
                        return colour_ < right.colour_;
                    }
                    return height_ < right.height_;
                }
                return width_ < right.width_;
            }
            return y_ < right.y_;
        }
        return x_ < right.x_;
    }
    bool operator==(Glitz const& other) const {
        return x_ == other.x_
            && y_ == other.y_
            && width_ == other.width_
            && height_ == other.height_
            && colour_ == other.colour_;
    }
private:
    int x_;
    int y_;
    int width_;
    int height_;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour_;

};
}//namespace hg
#endif //HG_GLITZ_H
