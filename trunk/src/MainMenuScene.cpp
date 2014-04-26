#include "MainMenuScene.h"
#include "Maths.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
namespace hg {
extern sf::Font const *defaultFont;
struct MenuItem {
    variant<RunALevel_tag, RunAReplay_tag, Exit_tag> tag;
    std::string text;
};


static void drawMainMenu(hg::RenderWindow &window, std::vector<MenuItem> const &menu, int const currentItem) {
    window.clear();
    int ypos = 100;
    int i = 0;
    for (auto const& menuItem: menu) {
        sf::Text menuItemGlyph;
        menuItemGlyph.setFont(*hg::defaultFont);
        menuItemGlyph.setString(menuItem.text);
        menuItemGlyph.setColor(i == currentItem ? sf::Color(0, 255, 255) : sf::Color(255,255,255));
        menuItemGlyph.setPosition(200, ypos);
        ypos += 50;
        menuItemGlyph.setCharacterSize(32.f);
        window.draw(menuItemGlyph);
        
        ++i;
    }
    window.display();
}

variant<RunALevel_tag, RunAReplay_tag, Exit_tag>
run_main_menu(hg::RenderWindow &window)
{
    int currentItem = 0;
    std::vector<MenuItem> const menu {
        {RunALevel_tag{}, "Run Level"},
        {RunAReplay_tag{}, "Run Replay"},
        {Exit_tag{}, "Exit"}
    };
    
    while (true) {
        drawMainMenu(window, menu, currentItem);
        bool mainMenuDrawn = true;
        while (mainMenuDrawn) {
            sf::Event event;
            do if (window.waitEvent(event)) {
                switch (event.type) {
                  case sf::Event::KeyPressed:
                    switch (event.key.code) {
                      case sf::Keyboard::Return:
                        return menu[currentItem].tag;
                        break;
                      case sf::Keyboard::Up:
                      case sf::Keyboard::W:
                        currentItem = flooredModulo(currentItem-1, static_cast<int>(menu.size()));
                        mainMenuDrawn = false;
                      break;
                      case sf::Keyboard::Down:
                      case sf::Keyboard::S:
                        currentItem = flooredModulo(currentItem+1, static_cast<int>(menu.size()));
                        mainMenuDrawn = false;
                      break;
                      default:
                        break;
                    }
                    break;
                  case sf::Event::Closed:
                    throw WindowClosed_exception{};
                  break;
                  case sf::Event::Resized:
                    mainMenuDrawn = false;
                  break;
                  default:
                    break;
                }
            } while (window.pollEvent(event));
        }
    }
}

}
