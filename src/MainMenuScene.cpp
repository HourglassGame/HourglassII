#include "MainMenuScene.h"
#include "Maths.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include <chrono>
#include <iostream>
namespace hg {
extern sf::Font const *defaultFont;
struct MenuItem final {
    std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag> tag;
    std::string text;
};


static void drawMainMenu(hg::RenderWindow &window, std::vector<MenuItem> const &menu, int const currentItem) {
    window.clear();
    float ypos = 200.f;
    int i = 0;
    for (auto const& menuItem: menu) {
        sf::Text menuItemGlyph;
        menuItemGlyph.setFont(*hg::defaultFont);
        menuItemGlyph.setString(menuItem.text);
        auto const menuItemColour = i == currentItem ? sf::Color(0, 255, 255) : sf::Color(255, 255, 255);
        menuItemGlyph.setFillColor(menuItemColour);
        menuItemGlyph.setOutlineColor(menuItemColour);
        menuItemGlyph.setPosition(450.f, ypos);
        ypos += 50.f;
        menuItemGlyph.setCharacterSize(42);
        window.draw(menuItemGlyph);
        
        ++i;
    }
    window.display();
}

std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag>
run_main_menu(hg::RenderWindow &window, VulkanEngine &vulkanEng)
{
    int currentItem = 0;
    std::vector<MenuItem> const menu {
        {RunALevel_tag{}, "Run Level"},
        {RunAReplay_tag{}, "Run Replay"},
        {Exit_tag{}, "Exit"}
    };
    auto frameStart = std::chrono::steady_clock::now();
    while (true) {
        drawMainMenu(window, menu, currentItem);

        bool mainMenuDrawn = true;
        while (mainMenuDrawn) {
            //vulkanEng.drawFrame();
            auto nextFrame = std::chrono::steady_clock::now();
            std::chrono::duration<double> frameTime = nextFrame - frameStart;
            //std::cout << std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count() << "\n";
            frameStart = nextFrame;
            sf::Event event;
            if (window.pollEvent(event)) do {
                switch (event.type) {
                  case sf::Event::KeyPressed:
                    switch (event.key.code) {
                      case sf::Keyboard::Return:
                        return menu[currentItem].tag;
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
