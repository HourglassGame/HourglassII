#include "SelectionScene.h"
#include <SFML/Graphics/Text.hpp>
namespace hg {
    static void drawOptionSelection(hg::RenderWindow &window, std::string const &selected) {
        //TODO: Display surrounding options, rather than just one option;
        //      Display some sort of title, explaining what is being selected.
        window.clear();

        sf::Text levelNameGlyph;
        levelNameGlyph.setFont(*hg::defaultFont);
        levelNameGlyph.setString(selected);
        levelNameGlyph.setColor(sf::Color(255, 255, 255));
        levelNameGlyph.setPosition(150, 120);
        levelNameGlyph.setCharacterSize(32);
        window.draw(levelNameGlyph);

        window.display();
    }

    hg::variant<std::size_t, SceneAborted_tag> run_selection_scene(hg::RenderWindow &window, std::vector<std::string> const &options)
    {
        if (options.empty()) //If no options available, just display a blank screen until the user escapes out.
        {
            window.clear();
            window.display();
            while (true) {
                sf::Event event;
                if (window.waitEvent(event)) do {
                    switch (event.type) {
                    case sf::Event::KeyPressed:
                        switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            return SceneAborted_tag{};
                            break;
                        default: break;
                        }
                        break;
                    case sf::Event::Closed:
                        throw WindowClosed_exception{};
                        break;
                    case sf::Event::Resized:
                        window.clear();
                        window.display();
                        break;
                    default:
                        break;
                    }
                } while (window.pollEvent(event));
            }
        }

        //TODO: Use size_t rather than int.
        int selectedItem = 0;

        while (true) {
            drawOptionSelection(window, options[selectedItem]);
            bool menuDrawn = true;
            while (menuDrawn) {
                sf::Event event;
                if (window.waitEvent(event)) do {
                    switch (event.type) {
                    case sf::Event::KeyPressed:
                        switch (event.key.code) {
                        case sf::Keyboard::Return:
                        {
                            return static_cast<std::size_t>(selectedItem);
                        }
                        case sf::Keyboard::Up:
                        case sf::Keyboard::W:
                            selectedItem = flooredModulo(selectedItem - 1, static_cast<int>(options.size()));
                            menuDrawn = false;
                            break;
                        case sf::Keyboard::Down:
                        case sf::Keyboard::S:
                            selectedItem = flooredModulo(selectedItem + 1, static_cast<int>(options.size()));
                            menuDrawn = false;
                            break;
                        case sf::Keyboard::Escape:
                            return SceneAborted_tag{};
                        default: break;
                        }
                        break;
                    case sf::Event::Closed:
                        throw WindowClosed_exception{};
                    case sf::Event::Resized:
                        menuDrawn = false;
                        break;
                    default:
                        break;
                    }
                } while (window.pollEvent(event));
            }
        }
    }
}
