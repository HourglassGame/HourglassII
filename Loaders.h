#ifndef HG_LOADERS_H
#define HG_LOADERS_H
#include <string>
namespace sf {
    class Image;
    class Font;
}

namespace hg {
    class Level;
    ::sf::Image loadImageAcceptFail(::std::string filename, int widthOnFail=32, int heightOnFail=32);
    ::sf::Font loadFontAcceptFail(::std::string filename);
    //Currently ignores filename and just returns a default level.
    //This behaviour is guaranteed for "" (empty string) argument but could change at any stage for other arguments
    Level loadLevelAcceptFail(const ::std::string& filename);
}
#endif //HG_LOADERS_H
