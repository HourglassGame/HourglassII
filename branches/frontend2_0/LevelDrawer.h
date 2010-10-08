#ifndef HG_LEVEL_DRAWER_H
#define HG_LEVEL_DRAWER_H
#include "TimeDirection.h"
#include <SFML/Graphics/Image.hpp>
namespace sf {
    class RenderTarget;
}
namespace hg {
class ObjectList;
class Level;
    class LevelDrawer {
    public:
        LevelDrawer(const Level& nlevel);
        //Draws the entire level to the render-target, which should have set set its view appropriately
        // prior to calling this (to get just a sub-portion or whatever)
        // maybe not the most efficient system; but seems pretty flexible, 
        //so keeping it like this until some sort of performance issues arise
        //Investigate adding a percentage blend mode
        void draw(::sf::RenderTarget& target, const ObjectList& objectList, TimeDirection viewDirection);
        void drawWall(::sf::RenderTarget& target);
    private:
        //Acts as a reference for the wall map to use, as well as holding the bitmaps and sounds and other resources
        //for the level
        const Level& level;
    };
}
#endif //HG_LEVEL_DRAWER_H
