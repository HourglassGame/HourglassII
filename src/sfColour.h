#ifndef HG_SF_COLOUR_H
#define HG_SF_COLOUR_H
#include <SFML/Graphics/Color.hpp>
namespace hg {
    //Interprets colour as |...|RRRRRRRR|GGGGGGGG|BBBBBBBB|--------|
    inline sf::Color interpretAsColour(unsigned colour)
    {
        return sf::Color((colour & 0xFF000000) >> 24, (colour & 0x00FF0000) >> 16, (colour & 0x0000FF00) >> 8);
    }
}

#endif
