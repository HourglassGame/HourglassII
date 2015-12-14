#include "RuntimeErrorScene.h"
#include <SFML/Graphics/Text.hpp>
#include "Scene.h"
namespace hg {
extern sf::Font const *defaultFont;
static void draw_runtime_error_scene(hg::RenderWindow &window, LuaError const &e) {
    window.clear();
    window.draw(sf::Text(boost::diagnostic_information(e), *defaultFont, 8));
    window.display();
}
void report_runtime_error(hg::RenderWindow &window, LuaError const &e)
{
    //TODO: Significantly improve the error reporting (Capture more info, and present it in a more understandable and useful fashion).
    //LuaError can be:
    // illegal call occurred in lua code (Currently this is the only place it is used)
    // lua produced invalid value
    
    //In each case, we want to report on:
    // What was wrong
    // The back trace (lua and c++)
    //(the c++ backtrace is a 'logical' backtrace, showing which call to
    //lua caused the problem, not the actual c++ call stack)

    //Also include details about how to report on the error so that it can be reproduced.
    // - Log file
    // - Replay File (if error occurred during level running, rather than level loading)
    // - Level Name/basicTriggerSystem/Game Version/etc

    //Print:
    // Lua Backtrace
    // 
    // Precise error
    //

    draw_runtime_error_scene(window, e);
    while (true) {
        sf::Event event;
        if (window.waitEvent(event)) do {
            switch (event.type) {
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case sf::Keyboard::Escape:
                    return;
                default: break;
                }
                break;
            case sf::Event::Closed:
                throw WindowClosed_exception{};
            case sf::Event::Resized:
                draw_runtime_error_scene(window, e);
                break;
            default:
                break;
            }
        } while (window.pollEvent(event));
    }
}
}