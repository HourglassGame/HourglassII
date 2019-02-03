#include "LoadingLevelScene.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include "LoadedLevel.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Color.hpp>
#include "RenderWindow.h"
#include "async.h"
namespace hg {

static void drawLoadingScreen(hg::RenderWindow &window) {
    sf::Text loadingGlyph;
    loadingGlyph.setFont(*hg::defaultFont);
    loadingGlyph.setString("Loading Level...");
    loadingGlyph.setFillColor(sf::Color(255,255,255));
    loadingGlyph.setOutlineColor(sf::Color(255, 255, 255));
    loadingGlyph.setPosition(520, 450);
    loadingGlyph.setCharacterSize(12);
    window.clear();
    window.draw(loadingGlyph);
    window.display();
}

static std::variant<hg::LoadedLevel, LoadingCanceled_tag>
displayLoadingScreen(
        hg::RenderWindow &window,
        boost::future<TimeEngine> &futureLoadedLevel,
        OperationInterrupter &interrupter,
        hg::move_function<LoadedLevel(TimeEngine &&)> const &resourceLoadFun)
{
    bool sceneDrawn = false;
    while (futureLoadedLevel.wait_for(boost::chrono::milliseconds(100)) != boost::future_status::ready) {
        if (!sceneDrawn) {
            drawLoadingScreen(window);
        }
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                switch(event.type) {
                case sf::Event::Closed:
                    interrupter.interrupt();
                    futureLoadedLevel.wait();
                    throw WindowClosed_exception{};
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {
                        interrupter.interrupt();
                        futureLoadedLevel.wait();
                        return LoadingCanceled_tag{};
                    }
                    break;
                case sf::Event::Resized:
                    sceneDrawn = false;
                break;
                default: break;
                }
            }
        }
    }
    auto loadedLevel = resourceLoadFun(futureLoadedLevel.get());
    if (interrupter.interrupted()) { return LoadingCanceled_tag{}; }
    return std::move(loadedLevel);
}

std::variant<hg::LoadedLevel, LoadingCanceled_tag>
load_level_scene(
        hg::RenderWindow &window,
        LoadLevelFunction const &levelLoadingFunction)
{
    hg::OperationInterrupter interruptor;

    auto futureLoadedLevel = async([&]() {return levelLoadingFunction.timeEngineLoadFun(interruptor); });

    return displayLoadingScreen(window, futureLoadedLevel, interruptor, levelLoadingFunction.glitzLoadFun);
}
}
