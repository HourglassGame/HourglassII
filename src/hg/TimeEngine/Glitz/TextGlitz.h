#ifndef HG_TEXT_GLITZ_H
#define HG_TEXT_GLITZ_H
#include "GlitzImplementation.h"
#include <boost/polymorphic_cast.hpp>
#include "hg/FrontEnd/LayeredCanvas.h"
#include "hg/mt/std/string"
#include <tuple>
namespace hg {
class TextGlitz final : public GlitzImplementation {
private:
	auto comparison_tuple() const {
		return std::tie(layer, text, x, y, size, colour);
	}
public:
	TextGlitz(
		int layer,
		mt::std::string text,
		int x, int y,
		int size,
		unsigned colour) :
			layer(layer),
			text(std::move(text)),
			x(x), y(y),
			size(size),
			colour(colour) {}
	virtual void display(LayeredCanvas &canvas) const override {
		std::string text_str(std::begin(text), std::end(text));
		canvas.drawText(layer, text_str, x/100.f, y/100.f, size/100.f, colour);
	}
	virtual std::size_t clone_size() const override {
		return sizeof *this;
	}
	virtual TextGlitz *perform_clone(void *memory) const override {
		return new (memory) TextGlitz(*this);
	}
	
	virtual bool operator<(GlitzImplementation const &right) const override {
		TextGlitz const &actual_right(*boost::polymorphic_downcast<TextGlitz const *>(&right));
		return comparison_tuple() < actual_right.comparison_tuple();
	}
	virtual bool operator==(GlitzImplementation const &o) const override {
		TextGlitz const &actual_other(*boost::polymorphic_downcast<TextGlitz const *>(&o));
		return comparison_tuple() == actual_other.comparison_tuple();
	}
private:
	virtual int order_ranking() const override {
		return 2;
	}
	int layer;
	
	mt::std::string text;
	
	int x;
	int y;
	
	int size;
	
	//Colour packed as |RRRRRRRR|GGGGGGGG|BBBBBBBB|*unused*|
	//Why? -- because lua is all ints, and I can't be bothered with a better interface for such a temporary thing.
	unsigned colour;
};
}
#endif //HG_TEXT_GLITZ_H
