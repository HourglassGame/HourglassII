#ifndef HG_GLITZ_H
#define HG_GLITZ_H
#include "LayeredCanvas.h"
#include <boost/operators.hpp>
#include <tuple>
#include "clone_ptr.h"
#include "memory_source_clone.h"
#include "multi_thread_memory_source.h"
#include "multi_thread_allocator.h"
#include "multi_thread_deleter.h"
#include "GlitzImplementation.h"
#include "mt/std/memory"
#include <cassert>
namespace hg {
class Glitz final : boost::totally_ordered<Glitz> {
public:
    explicit Glitz(mt::std::unique_ptr<GlitzImplementation> impl)
      : impl(impl.release()),
       _index(-1)
    {
        assert(this->impl);
    }
    explicit Glitz(mt::std::unique_ptr<GlitzImplementation> impl, int index)
      : impl(impl.release()),
       _index(index)
    {
        assert(this->impl);
    }
    
    void display(LayeredCanvas &canvas, int wantedIndex) const {
        if (_index == -1 || wantedIndex == _index)
        {
            impl->display(canvas);
        }
    }

    //Glitz has to be ordered (for sorting arrivals),
    //but since it can be pretty much
    //arbitrary drawing or other SFX/GFX (this is not yet implemented),
    //this ordering will too have to be pretty arbitrary unfortunately.
    //RectangleGlitz = 0
    //LineGlitz = 1
    //TextGlitz = 2
    //ImageGlitz = 3
    bool operator<(Glitz const &o) const {
        return comparison_tuple() < o.comparison_tuple();
    }
    bool operator==(Glitz const &o) const {
        return comparison_tuple() == o.comparison_tuple();
    }
private:

    clone_ptr<GlitzImplementation, memory_source_clone<GlitzImplementation, multi_thread_memory_source>> impl;
    typedef
      std::tuple<
        decltype(impl->order_ranking()),
        GlitzImplementation const &>
      comparison_tuple_type;
    comparison_tuple_type comparison_tuple() const {
        return comparison_tuple_type(impl->order_ranking(), *impl);
    }

    int _index;
};
}//namespace hg
#endif //HG_GLITZ_H
