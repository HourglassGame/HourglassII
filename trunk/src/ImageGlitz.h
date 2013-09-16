#ifndef HG_IMAGE_GLITZ_H
#define HG_IMAGE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/cast.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <string>
#include "LayeredCanvas.h"
namespace hg {
class ImageGlitz: public GlitzImplementation {
public:
    ImageGlitz(
        int layer,
        std::string const& key,
        int x, int y,
        int width, int height) :
            layer(layer), key(key),
            x(x), y(y),
            width(width), height(height)
    {}
    virtual void display(LayeredCanvas& canvas) const {
        canvas.drawImage(layer, key, x/100.f, y/100.f, width/100.f, height/100.f);
    }
    virtual std::size_t clone_size() const {
        return sizeof *this;
    }
    virtual ImageGlitz *perform_clone(void *memory) const {
        return new (memory) ImageGlitz(*this);
    }
    virtual bool operator<(GlitzImplementation const& right) const {
        ImageGlitz const& actual_right(*boost::polymorphic_downcast<ImageGlitz const*>(&right));
        return asTie() < actual_right.asTie();
    }
    virtual bool operator==(GlitzImplementation const& o) const {
        ImageGlitz const& actual_other(*boost::polymorphic_downcast<ImageGlitz const*>(&o));
        return asTie() == actual_other.asTie();
    }
private:
    boost::tuple<int const&, std::string const&, int const&, int const&, int const&, int const&> asTie() const {
        return boost::tie(layer, key, x, y, width, height);
    }
    virtual int order_ranking() const {
        return 3;
    }
    int layer;
    
    std::string key;
    
    int x;
    int y;
    int width;
    int height;
};
}
#endif //HG_IMAGE_GLITZ_H
