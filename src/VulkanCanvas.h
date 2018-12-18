#ifndef HG_VULKAN_CANVAS_H
#define HG_VULKAN_CANVAS_H
#include "ResourceManager.h"
#include "Canvas.h"
#include <tuple>
#include <string>
#include <iostream>
namespace hg {
    class VulkanCanvas final : public Canvas
    {
    public:
        explicit VulkanCanvas() :
            vertices()
        {}
        virtual void playSound(std::string const &key, int const n) override {
            //soundsToPlay.push_back(AudioGlitzObject(key, n));
        }
        virtual void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
        {
            float const left{x};
            float const right{x+width};
            float const top{y};
            float const bottom{y+height};

            vertices.emplace_back(left, top);
            vertices.emplace_back(right, top);
            vertices.emplace_back(right, bottom);

            vertices.emplace_back(right, bottom);
            vertices.emplace_back(left, bottom);
            vertices.emplace_back(left, top);
            /*
            sf::RectangleShape rect(sf::Vector2f(width, height));
            rect.setPosition(x, y);
            rect.setFillColor(interpretAsColour(colour));
            target->draw(rect);
            */
        }
        virtual void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
        {
            /*
            sf::Vector2f const pa(xa, ya);
            sf::Vector2f const pb(xb, yb);
            if (pa != pb) {
                sf::Vector2f d(normal(pa - pb)*(width / 2.f));
                sf::ConvexShape line;
                line.setPointCount(4);
                line.setPoint(0, pa - d);
                line.setPoint(1, pb - d);
                line.setPoint(2, pb + d);
                line.setPoint(3, pa + d);
                line.setFillColor(interpretAsColour(colour));
                target->draw(line);
            }
            */
        }
        virtual void drawText(std::string const &text, float const x, float const y, float const size, unsigned const colour) override
        {
            /*
            sf::Text glyphs;
            glyphs.setFont(*defaultFont);
            glyphs.setString(sf::String(text));
            glyphs.setPosition(x, y);
            glyphs.setCharacterSize(static_cast<unsigned>(size));
            glyphs.setFillColor(interpretAsColour(colour));
            glyphs.setOutlineColor(interpretAsColour(colour));
            target->draw(glyphs);
            */
        }
        virtual void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
        {
            drawRect(x, y, width, height, 0);
            /*
            auto it(resources->images.find(key));
            if (it == resources->images.end()) {
                std::cout << "Key: " << key << " not found\n";
                assert(it != resources->images.end());
            }
            sf::Texture tex;
            tex.loadFromImage(it->second);
            sf::Sprite sprite(tex);
            sprite.setPosition(sf::Vector2f(x, y));
            sprite.setScale(sf::Vector2f(width*1.f / tex.getSize().x, height*1.f / tex.getSize().y));
            target->draw(sprite);
            */
        }
        virtual void flushFrame() override {
            //audioPlayingState->runStep(audioGlitzManager->updatePlayingState(soundsToPlay));
        }
        std::vector<std::tuple<float, float>> vertices;
    private:
        /*
        sf::RenderTarget *target;
        AudioPlayingState *audioPlayingState;
        AudioGlitzManager *audioGlitzManager;
        LevelResources const *resources;
        std::vector<AudioGlitzObject> soundsToPlay;
        */
    };
    class PairCanvas final : public Canvas
    {
    public:
        explicit PairCanvas(Canvas &a, Canvas &b) :
            a(&a), b(&b)
        {}
        virtual void playSound(std::string const &key, int const n) override {
            a->playSound(key, n);
            b->playSound(key, n);
        }
        virtual void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
        {
            a->drawRect(x, y, width, height, colour);
            b->drawRect(x, y, width, height, colour);
        }
        virtual void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
        {
            a->drawLine(xa, ya, xb, yb, width, colour);
            b->drawLine(xa, ya, xb, yb, width, colour);
        }
        virtual void drawText(std::string const &text, float const x, float const y, float const size, unsigned const colour) override
        {
            a->drawText(text, x, y, size, colour);
            b->drawText(text, x, y, size, colour);
        }
        virtual void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
        {
            a->drawImage(key, x, y, width, height);
            b->drawImage(key, x, y, width, height);
        }
        virtual void flushFrame() override {
            a->flushFrame();
            b->flushFrame();
        }
    private:
        Canvas *a;
        Canvas *b;
    };
}
#endif //HG_VULKAN_CANVAS_H
