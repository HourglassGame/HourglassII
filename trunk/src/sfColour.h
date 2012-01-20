#ifndef HG_SF_COLOUR_H
#define HG_SF_COLOUR_H
#include <SFML/Graphics/Color.hpp>
namespace hg {
    //Interprets colour as |...|RRRRRRRR|GGGGGGGG|BBBBBBBB|--------|
    sf::Color interpretAsColour(unsigned colour)
    {
        return sf::Color((colour & 0xFF000000) >> 24, (colour & 0xFF0000) >> 16, (colour & 0xFF00) >> 8);
    }
}

#endif
