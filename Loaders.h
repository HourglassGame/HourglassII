#ifndef HG_LOADERS_H
#define HG_LOADERS_H
#include <string>
namespace sf {
    class Image;
    class Font;
}
namespace hg {
    ::sf::Image loadImageAcceptFail(::std::string filename, int widthOnFail=32, int heightOnFail=32);
    ::sf::Font loadFontAcceptFail(::std::string filename);
}
#endif //HG_LOADERS_H
