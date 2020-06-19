#ifndef HG_RECTANGLE_GLITZ_H
#define HG_RECTANGLE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/polymorphic_cast.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "hg/FrontEnd/LayeredCanvas.h"
namespace hg {
class RectangleGlitz final : public GlitzImplementation {
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
	virtual void display(LayeredCanvas &canvas) const override {
		canvas.drawRect(layer, x/100.f, y/100.f, width/100.f, height/100.f, colour);
	}
	virtual std::size_t clone_size() const override {
		return sizeof *this;
	}
	virtual RectangleGlitz *perform_clone(void *memory) const override {
		return new (memory) RectangleGlitz(*this);
	}
	virtual bool operator<(GlitzImplementation const &right) const override {
		RectangleGlitz const &actual_right(*boost::polymorphic_downcast<RectangleGlitz const*>(&right));
		return asTie() < actual_right.asTie();
	}
	virtual bool operator==(GlitzImplementation const &o) const override {
		RectangleGlitz const &actual_other(*boost::polymorphic_downcast<RectangleGlitz const*>(&o));
		return asTie() == actual_other.asTie();
	}
private:
	boost::tuple<int const &, int const &, int const &, int const &, int const &, unsigned const &> asTie() const {
		return boost::tie(layer, x, y, width, height, colour);
	}
	virtual int order_ranking() const override {
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
