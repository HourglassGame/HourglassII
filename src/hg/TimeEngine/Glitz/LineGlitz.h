#ifndef HG_LINE_GLITZ_H
#define HG_LINE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/polymorphic_cast.hpp>
#include "hg/FrontEnd/LayeredCanvas.h"
#include <tuple>
namespace hg {
class LineGlitz final : public GlitzImplementation {
private:
	auto comparison_tuple() const {
		return std::tie(layer, xa, ya, xb, yb, width, colour);
	}
public:
	LineGlitz(
		int layer,
		int xa, int ya,
		int xb, int yb,
		int width,
		unsigned colour) :
			layer(layer),
			xa(xa), ya(ya),
			xb(xb), yb(yb),
			width(width),
			colour(colour) {}
	virtual void display(LayeredCanvas &canvas) const override {
		canvas.drawLine(layer, xa/100.f, ya/100.f, xb/100.f, yb/100.f, width/100.f, colour);
	}
	virtual std::size_t clone_size() const override {
		return sizeof *this;
	}
	virtual LineGlitz *perform_clone(void *memory) const override {
		return new (memory) LineGlitz(*this);
	}
	
	virtual std::strong_ordering operator<=>(GlitzImplementation const &right) const override {
		LineGlitz const &actual_right(*boost::polymorphic_downcast<LineGlitz const *>(&right));
		return comparison_tuple() <=> actual_right.comparison_tuple();
	}
	virtual bool operator==(GlitzImplementation const &o) const override {
		LineGlitz const &actual_other(*boost::polymorphic_downcast<LineGlitz const *>(&o));
		return comparison_tuple() == actual_other.comparison_tuple();
	}
private:
	virtual int order_ranking() const override {
		return 1;
	}
	int layer;
	
	int xa;
	int ya;
	int xb;
	int yb;
	
	int width;
	
	//Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
	//Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
	unsigned colour;
};
}
#endif
