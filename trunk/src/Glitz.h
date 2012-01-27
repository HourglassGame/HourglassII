#ifndef HG_GLITZ_H
#define HG_GLITZ_H
#include "Canvas.h"
#include <boost/operators.hpp>
#include <boost/move/move.hpp>
#include "clone_ptr.h"
#include "memory_source_clone.h"
#include "multi_thread_memory_source.h"
#include "GlitzImplementation.h"
#include <cassert>
namespace hg {
class Glitz : boost::totally_ordered<Glitz> {
public:
    Glitz(Glitz const& other) :
        x_(other.x_), y_(other.y_),
        width_(other.width_), height_(other.height_),
        colour_(other.colour_),
        impl_(other.impl_)
    {}

    Glitz(BOOST_RV_REF(Glitz) other) :
        x_(other.x_), y_(other.y_),
        width_(other.width_), height_(other.height_),
        colour_(other.colour_),
        impl_(boost::move(other.impl_))
    {}
    
    Glitz& operator=(BOOST_COPY_ASSIGN_REF(Glitz) other) {
        x_ = other.x_;
        y_ = other.y_;
        width_ = other.width_;
        height_ = other.height_;
        colour_ = other.colour_;
        impl_ = other.impl_;
        return *this;
    }
    Glitz& operator=(BOOST_RV_REF(Glitz) other) {
        x_ = boost::move(other.x_);
        y_ = boost::move(other.y_);
        width_ = boost::move(other.width_);
        height_ = boost::move(other.height_);
        colour_ = boost::move(other.colour_);
        impl_ = boost::move(other.impl_);
        return *this;
    }

    Glitz(GlitzImplementation* impl) :impl_(impl){
        assert(impl);
    }
    
    /*Glitz(
        int x, int y,
        int width, int height,
        unsigned colour) :
            x_(x), y_(y),
            width_(width), height_(height),
            colour_(colour) {}*/
    
    void display(Canvas& canvas) const {
        impl_->display(canvas);
        //canvas.drawRect(x_/100., y_/100., width_/100., height_/100., colour_);
    }

    //Glitz has to be ordered (for sorting arrivals),
    //but since it can be pretty much
    //arbitrary drawing or other SFX/GFX (this is not yet implemented),
    //this ordering will too have to be pretty arbitrary unfortunately.
    bool operator<(Glitz const& right) const {
        if (impl_->order_ranking() == right.impl_->order_ranking()) {
            return *impl_ < *right.impl_;
        }
        return impl_->order_ranking() < right.impl_->order_ranking();
    /*
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
        */
    }
    bool operator==(Glitz const& other) const {
        return impl_->order_ranking() == other.impl_->order_ranking()
            && *impl_ == *other.impl_;
        
        /*
        return x_ == other.x_
            && y_ == other.y_
            && width_ == other.width_
            && height_ == other.height_
            && colour_ == other.colour_;
            */
    }
private:
    int x_;
    int y_;
    int width_;
    int height_;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour_;


    clone_ptr<GlitzImplementation, memory_source_clone<GlitzImplementation, multi_thread_memory_source> > impl_;
    BOOST_COPYABLE_AND_MOVABLE(Glitz)
};
}//namespace hg
#endif //HG_GLITZ_H
