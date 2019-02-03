#ifndef HG_SF_RENDER_TARGET_CANVAS_H
#define HG_SF_RENDER_TARGET_CANVAS_H
#include "ResourceManager.h"
#include "Canvas.h"
#include "sfColour.h"
#include "Maths.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <iostream>
namespace hg {


class sfRenderTargetVisualCanvas final : public Canvas
{
public:
    explicit sfRenderTargetVisualCanvas(
        sf::RenderTarget &target,
        LevelResources const &resources) :
    target(&target),
    resources(&resources)
    {}
    virtual void playSound(std::string const &key, int const n) override {
        (void)key; (void)n;
    }
    virtual void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
    {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(x, y);
        rect.setFillColor(interpretAsColour(colour));
        target->draw(rect);
    }
    virtual void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
    {
        sf::Vector2f const pa(xa, ya);
        sf::Vector2f const pb(xb, yb);
        if (pa != pb) {
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
    }
    virtual void drawText(std::string const &text, float const x, float const y, float const size, unsigned const colour) override
    {
        sf::Text glyphs;
        glyphs.setFont(*defaultFont);
        glyphs.setString(sf::String(text));
        glyphs.setPosition(x, y);
        glyphs.setCharacterSize(static_cast<unsigned>(size));
        glyphs.setFillColor(interpretAsColour(colour));
        glyphs.setOutlineColor(interpretAsColour(colour));
        target->draw(glyphs);
    }
    virtual void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
    {
        auto it(resources->images.find(key));
        if (it == resources->images.end()) {
            std::cout << "Key: " << key << " not found\n";
            assert(it != resources->images.end());
        }
        sf::Texture tex;
        tex.loadFromImage(it->second);
        sf::Sprite sprite(tex);
        sprite.setPosition(sf::Vector2f(x,y));
        sprite.setScale(sf::Vector2f(width*1.f/tex.getSize().x, height*1.f/tex.getSize().y));
        target->draw(sprite);
    }
    virtual void flushFrame() override {
    }
private:
    sf::RenderTarget *target;
    LevelResources const *resources;
};


class sfRenderTargetAudioCanvas final : public Canvas
{
public:
    explicit sfRenderTargetAudioCanvas(
        AudioPlayingState &audioPlayingState,
        AudioGlitzManager &audioGlitzManager) :
    audioPlayingState(&audioPlayingState),
    audioGlitzManager(&audioGlitzManager),
    soundsToPlay()
    {}
    virtual void playSound(std::string const &key, int const n) override {
        soundsToPlay.push_back(AudioGlitzObject(key,n));
    }
    virtual void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
    {
        (void)x; (void)y; (void)width; (void)height; (void)colour;
    }
    virtual void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
    {
        (void)xa; (void)ya; (void)xb; (void)yb; (void)width; (void)colour;
    }
    virtual void drawText(std::string const &text, float const x, float const y, float const size, unsigned const colour) override
    {
        (void)text; (void)x; (void)y; (void)size; (void)colour;
    }
    virtual void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
    {
        (void)key; (void)x; (void)y; (void)width; (void)height;
    }
    virtual void flushFrame() override {
        audioPlayingState->runStep(audioGlitzManager->updatePlayingState(soundsToPlay));
    }
private:
    AudioPlayingState *audioPlayingState;
    AudioGlitzManager *audioGlitzManager;
    std::vector<AudioGlitzObject> soundsToPlay;
};

}
#endif //HG_SF_RENDER_TARGET_CANVAS_H
