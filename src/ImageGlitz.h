#ifndef HG_IMAGE_GLITZ_H
#define HG_IMAGE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/polymorphic_cast.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "mt/std/string"
#include "LayeredCanvas.h"
namespace hg {
class ImageGlitz final : public GlitzImplementation {
    auto asTie() const -> decltype(auto) {
        return boost::tie(layer, key, x, y, width, height);
    }
public:
    ImageGlitz(
        int layer,
        mt::std::string key,
        int x, int y,
        int width, int height) :
            layer(layer), key(std::move(key)),
            x(x), y(y),
            width(width), height(height)
    {}
    virtual void display(LayeredCanvas &canvas) const override {
        std::string key_str(std::begin(key), std::end(key));
        canvas.drawImage(layer, key_str, x/100.f, y/100.f, width/100.f, height/100.f);
    }
    virtual std::size_t clone_size() const override {
        return sizeof *this;
    }
    virtual ImageGlitz *perform_clone(void *memory) const override {
        return new (memory) ImageGlitz(*this);
    }
    virtual bool operator<(GlitzImplementation const &right) const override {
        ImageGlitz const &actual_right(*boost::polymorphic_downcast<ImageGlitz const*>(&right));
        return asTie() < actual_right.asTie();
    }
    virtual bool operator==(GlitzImplementation const &o) const override {
        ImageGlitz const &actual_other(*boost::polymorphic_downcast<ImageGlitz const*>(&o));
        return asTie() == actual_other.asTie();
    }
private:
    virtual int order_ranking() const override {
        return 3;
    }
    int layer;
    //TODO: investigate using small fixed maximum-size key, to avoid allocations
    mt::std::string key;
    
    int x;
    int y;
    int width;
    int height;
};
}
#endif //HG_IMAGE_GLITZ_H
