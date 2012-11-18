#ifndef HG_SF_RENDER_TARGET_CANVAS_H
#define HG_SF_RENDER_TARGET_CANVAS_H
#include "Canvas.h"
#include "sfColour.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shape.hpp>
namespace hg {

class sfRenderTargetCanvas : public Canvas
{
public:
    explicit sfRenderTargetCanvas(sf::RenderTarget& target) :
    target_(&target)
    {}
    virtual void drawRect(int x, int y, int width, int height, unsigned colour)
    {
        target_->Draw(sf::Shape::Rectangle(x, y, x + width, y + height, interpretAsColour(colour)));
    }
    virtual void drawLine(int xa, int ya, int xb, int yb, unsigned colour)
    {
        target_->Draw(sf::Shape::Line(xa, ya, xb, yb, 1.f, interpretAsColour(colour)));
    }
private:
    sf::RenderTarget* target_;
};
}
#endif //HG_SF_RENDER_TARGET_CANVAS_H
