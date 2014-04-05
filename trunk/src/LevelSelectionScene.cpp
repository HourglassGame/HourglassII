#include "LevelSelectionScene.h"
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/filesystem.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include "Maths.h"
#include "natural_sort.h"
namespace hg {

static void drawLevelSelection(hg::RenderWindow &window, std::string const& levelName) {
    window.clear();

    sf::Text levelNameGlyph;
    levelNameGlyph.setFont(*hg::defaultFont);
    levelNameGlyph.setString(levelName);
    levelNameGlyph.setColor(sf::Color(255,255,255));
    levelNameGlyph.setPosition(150, 120);
    levelNameGlyph.setCharacterSize(32.f);
    window.draw(levelNameGlyph);

    window.display();
}


variant<hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)>, WindowClosed_tag, SceneAborted_tag> run_level_selection_scene(hg::RenderWindow &window) {
    std::vector<boost::filesystem::path> levelPaths;
    for (auto entry: boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"), boost::filesystem::directory_iterator())) {
        if (is_directory(entry.status()) && entry.path().extension()==".lvl") {
            levelPaths.push_back(entry.path());
        }
    }
    
    boost::sort(
        levelPaths,
        [](boost::filesystem::path const& l,boost::filesystem::path const& r)
        {
            return natural_less(l.stem().string(), r.stem().string());
        });
    
    int selectedLevel = 0;
    
    while (true) {
        drawLevelSelection(window, levelPaths[selectedLevel].stem().string());
        bool menuDrawn = true;
        while (menuDrawn) {
            sf::Event event;
            do if (window.waitEvent(event)) {
                switch (event.type) {
                  case sf::Event::KeyPressed:
                    switch (event.key.code) {
                      case sf::Keyboard::Return:
                        return move_function<LoadedLevel(hg::OperationInterrupter &)>(
                            std::bind(loadLevelAndResourcesFromFile, levelPaths[selectedLevel], std::placeholders::_1));
                      case sf::Keyboard::Up:
                      case sf::Keyboard::W:
                        selectedLevel = flooredModulo(selectedLevel-1, static_cast<int>(levelPaths.size()));
                        menuDrawn = false;
                      break;
                      case sf::Keyboard::Down:
                      case sf::Keyboard::S:
                        selectedLevel = flooredModulo(selectedLevel+1, static_cast<int>(levelPaths.size()));
                        menuDrawn = false;
                      break;
                      case sf::Keyboard::Escape:
                        return SceneAborted_tag{};
                      break;
                      default: break;
                    }
                    break;
                  case sf::Event::Closed:
                    return WindowClosed_tag{};
                  break;
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
