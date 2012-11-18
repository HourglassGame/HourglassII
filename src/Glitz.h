#ifndef HG_GLITZ_H
#define HG_GLITZ_H
#include "LayeredCanvas.h"
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
        impl(o.impl)
    {}

    Glitz(BOOST_RV_REF(Glitz) o) :
        impl(boost::move(o.impl))
    {}
    
    Glitz& operator=(BOOST_COPY_ASSIGN_REF(Glitz) o) {
        impl = o.impl;
        return *this;
    }
    Glitz& operator=(BOOST_RV_REF(Glitz) o) {
        impl = boost::move(o.impl);
        return *this;
    }

    explicit Glitz(GlitzImplementation* impl)
      : impl(impl)
    {
        assert(impl);
    }
    
    void display(LayeredCanvas& canvas) const {
        impl->display(canvas);
    }

    //Glitz has to be ordered (for sorting arrivals),
    //but since it can be pretty much
    //arbitrary drawing or other SFX/GFX (this is not yet implemented),
    //this ordering will too have to be pretty arbitrary unfortunately.
    bool operator<(Glitz const& right) const {
        if (impl->order_ranking() == right.impl->order_ranking()) {
            return *impl < *right.impl;
        }
        return impl->order_ranking() < right.impl->order_ranking();
    }
    bool operator==(Glitz const& o) const {
        return
               impl->order_ranking() == o.impl->order_ranking()
            && *impl == *o.impl;
    }
private:
    clone_ptr<GlitzImplementation, memory_source_clone<GlitzImplementation, multi_thread_memory_source> > impl;
    BOOST_COPYABLE_AND_MOVABLE(Glitz)
};
}//namespace hg
#endif //HG_GLITZ_H
