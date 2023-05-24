#ifndef HG_CIRCLE_GLITZ_H
#define HG_CIRCLE_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/polymorphic_cast.hpp>
#include "hg/FrontEnd/LayeredCanvas.h"
#include <tuple>
namespace hg {
class CircleGlitz final : public GlitzImplementation {
private:
	auto comparison_tuple() const {
		return std::tie(layer, x, y, radius, colour);
	}
public:
	CircleGlitz(
		int layer,
		int x, int y,
		int radius,
		unsigned colour) :
			layer(layer),
			x(x), y(y),
			radius(radius),
			colour(colour) {}
	virtual void display(LayeredCanvas &canvas) const override {
		canvas.drawCircle(layer, x/100.f, y/100.f, radius/100.f, colour);
	}
	virtual std::size_t clone_size() const override {
		return sizeof *this;
	}
	virtual CircleGlitz *perform_clone(void *memory) const override {
		return new (memory) CircleGlitz(*this);
	}
	virtual std::strong_ordering operator<=>(GlitzImplementation const &right) const override {
		CircleGlitz const &actual_right(*boost::polymorphic_downcast<CircleGlitz const*>(&right));
		return comparison_tuple() <=> actual_right.comparison_tuple();
	}
	virtual bool operator==(GlitzImplementation const &o) const override {
		CircleGlitz const &actual_other(*boost::polymorphic_downcast<CircleGlitz const*>(&o));
		return comparison_tuple() == actual_other.comparison_tuple();
	}
private:
	virtual int order_ranking() const override {
		return 4;
	}
	int layer;

	int x;
	int y;
	int radius;

	//Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
	//Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
	unsigned colour;
};
}
#endif //HG_CIRCLE_GLITZ_H
