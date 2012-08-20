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
    Glitz(Glitz const& o) :
        x_(o.x_), y_(o.y_),
        width_(o.width_), height_(o.height_),
        colour_(o.colour_),
        impl_(o.impl_)
    {}

    Glitz(BOOST_RV_REF(Glitz) o) :
        x_(o.x_), y_(o.y_),
        width_(o.width_), height_(o.height_),
        colour_(o.colour_),
        impl_(boost::move(o.impl_))
    {}
    
    Glitz& operator=(BOOST_COPY_ASSIGN_REF(Glitz) o) {
        x_ = o.x_;
        y_ = o.y_;
        width_ = o.width_;
        height_ = o.height_;
        colour_ = o.colour_;
        impl_ = o.impl_;
        return *this;
    }
    Glitz& operator=(BOOST_RV_REF(Glitz) o) {
        x_ = boost::move(o.x_);
        y_ = boost::move(o.y_);
        width_ = boost::move(o.width_);
        height_ = boost::move(o.height_);
        colour_ = boost::move(o.colour_);
        impl_ = boost::move(o.impl_);
        return *this;
    }

    explicit Glitz(GlitzImplementation* impl) :impl_(impl){
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
    bool operator==(Glitz const& o) const {
        return impl_->order_ranking() == o.impl_->order_ranking()
            && *impl_ == *o.impl_;
        
        /*
        return x_ == o.x_
            && y_ == o.y_
            && width_ == o.width_
            && height_ == o.height_
            && colour_ == o.colour_;
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
