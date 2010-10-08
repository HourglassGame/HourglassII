#include "Loaders.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <iostream>
using namespace ::sf;
using namespace ::std;
namespace hg {
    Image loadImageAcceptFail(string filename, int widthOnFail, int heightOnFail)
    {
        Image retv;
        if (!retv.LoadFromFile(filename)) {
            retv.Create(widthOnFail, heightOnFail, Color(255,0,255));
        }
        return retv;
    }
    Font loadFontAcceptFail(string filename)
    {
        Font retv;
        retv.LoadFromFile(filename);
        return retv;
    }
}

