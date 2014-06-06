#ifndef HG_GLITZ_IMPLEMENTATION_H
#define HG_GLITZ_IMPLEMENTATION_H
#include "unique_ptr.h"
namespace hg {
class LayeredCanvas;
class GlitzImplementation {
public:
    virtual void display(LayeredCanvas &canvas) const = 0;
    virtual std::size_t clone_size() const = 0;
    virtual GlitzImplementation *perform_clone(void *memory) const = 0;

    virtual ~GlitzImplementation() noexcept {}
    
    //Each glitz-implementation must provide a
    //unique return value for order_ranking.
    //This is used to sort glitz for arrival/departure analysis
    //(for persistent glitz).
    //AudioGlitz: -1
    //RectangleGlitz: 0
    //LineGlitz: 1
    //TextGlitz: 2
    //ImageGlitz: 3
    virtual int order_ranking() const = 0;
    virtual bool operator<(GlitzImplementation const &o) const = 0;
    virtual bool operator==(GlitzImplementation const &o) const = 0;
};
}


#endif //HG_GLITZ_IMPLEMENTATION_H
