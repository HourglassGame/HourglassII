#ifndef HG_SF_RENDER_TARGET_CANVAS_H
#define HG_SF_RENDER_TARGET_CANVAS_H
#include "ResourceManager.h"
#include "Canvas.h"
#include "sfColour.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
namespace hg {


namespace {
float magnitude(sf::Vector2f vec) {
    return std::sqrt(vec.x*vec.x+vec.y*vec.y);
}

sf::Vector2f normal(sf::Vector2f vec) {
    sf::Vector2f direction(vec.y, -vec.x);
    assert(magnitude(direction));
    return direction/magnitude(direction);
}
}

class sfRenderTargetCanvas : public Canvas
{
public:
    explicit sfRenderTargetCanvas(sf::RenderTarget& target, LevelResources const& resources) :
    target(&target), resources(&resources)
    {}
    virtual void drawRect(float x, float y, float width, float height, unsigned colour)
    {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(x, y);
        rect.setFillColor(interpretAsColour(colour));
        target->draw(rect);
    }
    virtual void drawLine(float xa, float ya, float xb, float yb, float width, unsigned colour)
    {
        sf::Vector2f const pa(xa, ya);
        sf::Vector2f const pb(xb, yb);
        sf::Vector2f d(normal(pa-pb)*(width/2.f));
        sf::ConvexShape line;
        line.setPointCount(4);
        line.setPoint(0, pa - d);
        line.setPoint(1, pb - d);
        line.setPoint(2, pb + d);
        line.setPoint(3, pa + d);
        line.setFillColor(interpretAsColour(colour));
        target->draw(line);
    }
    virtual void drawText(std::string const& text, float x, float y, float size, unsigned colour)
    {
        sf::Text glyphs;
        glyphs.setFont(*defaultFont);
        glyphs.setString(sf::String(text));
        glyphs.setPosition(x, y);
        glyphs.setCharacterSize(size);
        glyphs.setColor(interpretAsColour(colour));
        target->draw(glyphs);
    }
    virtual void drawImage(std::string const& key, float x, float y, float width, float height) {
        std::map<std::string, sf::Image>::const_iterator it(resources->images.find(key));
        assert(it != resources->images.end());
        sf::Texture tex;
        tex.loadFromImage(it->second);
        sf::Sprite sprite(tex);
        sprite.setPosition(sf::Vector2f(x,y));
        sprite.setScale(sf::Vector2f(width*1.f/tex.getSize().x, height*1.f/tex.getSize().y));
        target->draw(sprite);
    }
private:
    sf::RenderTarget *target;
    LevelResources const *resources;
};
}
#endif //HG_SF_RENDER_TARGET_CANVAS_H
