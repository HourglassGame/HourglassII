#ifndef HG_LINE_GLITZ_H
#define HG_LINE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/cast.hpp>
namespace hg {
class LineGlitz: public GlitzImplementation {
public:
    LineGlitz(
        int layer,
        int xa, int ya,
        int xb, int yb,
        unsigned colour) :
            layer(layer),
            xa(xa), ya(ya),
            xb(xb), yb(yb),
            colour(colour) {}
    virtual void display(LayeredCanvas& canvas) const {
        canvas.drawLine(layer, xa/100., ya/100., xb/100., yb/100., colour);
    }
    virtual std::size_t clone_size() const {
        return sizeof(*this);
    }
    virtual LineGlitz* perform_clone(void* memory) const {
        return new (memory) LineGlitz(*this);
    }
    virtual bool operator<(GlitzImplementation const& right) const {
        LineGlitz const& actual_right(*boost::polymorphic_downcast<LineGlitz const*>(&right));
        if (layer == actual_right.layer) {
            if (xa == actual_right.xa) {
                if (ya == actual_right.ya) {
                    if (xb == actual_right.xb) {
                        if (yb == actual_right.yb) {
                            return colour < actual_right.colour;
                        }
                        return yb < actual_right.yb;
                    }
                    return xb < actual_right.xb;
                }
                return ya < actual_right.ya;
            }
            return xa < actual_right.xa;
        }
        return layer < actual_right.layer;
    }
    virtual bool operator==(GlitzImplementation const& o) const {
        LineGlitz const& actual_other(*boost::polymorphic_downcast<LineGlitz const*>(&o));
        return layer == actual_other.layer
            && xa == actual_other.xa
            && ya == actual_other.ya
            && xb == actual_other.xb
            && yb == actual_other.yb
            && colour == actual_other.colour;
    }
private:
    virtual int order_ranking() const {
        return 1;
    }
    int layer;
    
    int xa;
    int ya;
    int xb;
    int yb;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour;
};
}
#endif
