#include "LoadingLevelScene.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include "LoadedLevel.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include "RenderWindow.h"
namespace hg {

static void drawLoadingScreen(hg::RenderWindow &window) {
    sf::Text loadingGlyph;
    loadingGlyph.setFont(*hg::defaultFont);
    loadingGlyph.setString("Loading Level...");
    loadingGlyph.setColor(sf::Color(255,255,255));
    loadingGlyph.setPosition(520, 450);
    loadingGlyph.setCharacterSize(12.f);
    window.clear();
    window.draw(loadingGlyph);
    window.display();
}

static hg::variant<hg::LoadedLevel, LoadingCanceled_tag>
displayLoadingScreen(
        hg::RenderWindow &window,
        boost::future<LoadedLevel> &futureLoadedLevel,
        OperationInterrupter &interrupter)
{
    bool sceneDrawn = false;
    while (!futureLoadedLevel.is_ready()) {
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
        sf::sleep(sf::seconds(.1f));
    }
    if (interrupter.interrupted()) { return LoadingCanceled_tag{}; }
    return futureLoadedLevel.get();
}

hg::variant<hg::LoadedLevel, LoadingCanceled_tag>
load_level_scene(
        hg::RenderWindow &window,
        hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)> const &levelLoadingFunction)
{
    hg::OperationInterrupter interruptor;
    tbb::structured_task_group group;
    boost::packaged_task<hg::LoadedLevel()> levelLoadingTask([&](){return levelLoadingFunction(interruptor);});
    auto futureLoadedLevel = levelLoadingTask.get_future();
    tbb::task_handle<decltype(std::ref(levelLoadingTask))> loadingLevel_task_handle{std::ref(levelLoadingTask)};
    group.run(loadingLevel_task_handle);
    
    return displayLoadingScreen(window, futureLoadedLevel, interruptor);
}
}
