#ifndef HG_RECTANGLE_GLITZ_H
#define HG_RECTANGLE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/cast.hpp>
#include "LayeredCanvas.h"
namespace hg {
class RectangleGlitz: public GlitzImplementation {
public:
    RectangleGlitz(
        int layer,
        int x, int y,
        int width, int height,
        unsigned colour) :
            layer(layer),
            x(x), y(y),
            width(width), height(height),
            colour(colour) {}
    virtual void display(LayeredCanvas& canvas) const {
        canvas.drawRect(layer, x/100., y/100., width/100., height/100., colour);
    }
    virtual std::size_t clone_size() const {
        return sizeof *this;
    }
    virtual RectangleGlitz* perform_clone(void* memory) const {
        return new (memory) RectangleGlitz(*this);
    }
    virtual bool operator<(GlitzImplementation const& right) const {
        RectangleGlitz const& actual_right(*boost::polymorphic_downcast<RectangleGlitz const*>(&right));
        if (layer == actual_right.layer) {
            if (x == actual_right.x) {
                if (y == actual_right.y) {
                    if (width == actual_right.width) {
                        if (height == actual_right.height) {
                            return colour < actual_right.colour;
                        }
                        return height < actual_right.height;
                    }
                    return width < actual_right.width;
                }
                return y < actual_right.y;
            }
            return x < actual_right.x;
        }
        return layer < actual_right.layer;
    }
    virtual bool operator==(GlitzImplementation const& o) const {
        RectangleGlitz const& actual_other(*boost::polymorphic_downcast<RectangleGlitz const*>(&o));
        return layer == actual_other.layer
            && x == actual_other.x
            && y == actual_other.y
            && width == actual_other.width
            && height == actual_other.height
            && colour == actual_other.colour;
    }
private:
    virtual int order_ranking() const {
        return 0;
    }
    int layer;
    
    int x;
    int y;
    int width;
    int height;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour;
};
}
#endif //HG_RECTANGLE_GLITZ_H
