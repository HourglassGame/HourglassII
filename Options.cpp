#include "Options.h"
#include <SFML/Window/WindowStyle.hpp>
namespace hg {
Options::Options() :
width(640),
height(480),
displaysplash(true),
windowStyle(/*::sf::Style::Fullscreen*/::sf::Style::Resize|::sf::Style::Close)
{
}
}
