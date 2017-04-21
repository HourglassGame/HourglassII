#ifndef HG_TEXT_GLITZ_H
#define HG_TEXT_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/cast.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "LayeredCanvas.h"
#include <string>
namespace hg {
class TextGlitz final : public GlitzImplementation {
private:
    auto asTie() const -> decltype(auto) {
        return boost::tie(layer, text, x, y, size, colour);
    }
public:
    TextGlitz(
        int layer,
        std::string text,
        int x, int y,
        int size,
        unsigned colour) :
            layer(layer),
            text(text),
            x(x), y(y),
            size(size),
            colour(colour) {}
    virtual void display(LayeredCanvas &canvas) const override {
        canvas.drawText(layer, text, x/100.f, y/100.f, size/100.f, colour);
    }
    virtual std::size_t clone_size() const override {
        return sizeof *this;
    }
    virtual TextGlitz *perform_clone(void *memory) const override {
        return new (memory) TextGlitz(*this);
    }
    
    virtual bool operator<(GlitzImplementation const &right) const override {
        TextGlitz const &actual_right(*boost::polymorphic_downcast<TextGlitz const *>(&right));
        return asTie() < actual_right.asTie();
    }
    virtual bool operator==(GlitzImplementation const &o) const override {
        TextGlitz const &actual_other(*boost::polymorphic_downcast<TextGlitz const *>(&o));
        return asTie() == actual_other.asTie();
    }
private:
    virtual int order_ranking() const override {
        return 2;
    }
    int layer;
    
    std::string text;
    
    int x;
    int y;
    
    int size;
    
    //Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
    //Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
    unsigned colour;
};
}
#endif //HG_TEXT_GLITZ_H
