#ifndef HG_SF_RENDER_TARGET_CANVAS_H
#define HG_SF_RENDER_TARGET_CANVAS_H
#include "Canvas.h"
#include "sfColour.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/String.hpp>
namespace hg {

class sfRenderTargetCanvas : public Canvas
{
public:
    explicit sfRenderTargetCanvas(sf::RenderTarget& target) :
    target_(&target)
    {}
    virtual void drawRect(float x, float y, float width, float height, unsigned colour)
    {
        target_->Draw(sf::Shape::Rectangle(x, y, x + width, y + height, interpretAsColour(colour)));
    }
    virtual void drawLine(float xa, float ya, float xb, float yb, float width, unsigned colour)
    {
        target_->Draw(sf::Shape::Line(xa, ya, xb, yb, width, interpretAsColour(colour)));
    }
    virtual void drawText(std::string const& text, float x, float y, float size, unsigned colour)
    {
        sf::String glyphs(text);
        glyphs.SetPosition(x, y);
        glyphs.SetSize(size);
        glyphs.SetColor(interpretAsColour(colour));
        target_->Draw(glyphs);
    }
private:
    sf::RenderTarget* target_;
};
}
#endif //HG_SF_RENDER_TARGET_CANVAS_H
