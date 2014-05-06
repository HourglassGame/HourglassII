#include "ResourceManager.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
namespace hg {

namespace fs = boost::filesystem;
namespace {
inline boost::iterator_range<fs::directory_iterator>
directory_range(fs::path const &path)
{
    return boost::iterator_range<fs::directory_iterator>(
        fs::directory_iterator(path),
        fs::directory_iterator());
}

inline bool isImageFile(fs::path const &file) {
    return file.extension() == ".png";
}

inline sf::Image loadImage(fs::path const &file) {
    sf::Image img;
	bool loaded(img.loadFromFile(file.string()));
    assert(loaded);
    return img;
}

inline sf::Texture loadTexture(fs::path const &file) {
    sf::Texture img;
	bool loaded(img.loadFromFile(file.string()));
    assert(loaded);
    return img;
}

inline void loadPackage(LevelResources &resourceStore, std::string const &prefix, std::string const &directory) {
    fs::path package_directory(directory);
    assert(fs::is_directory(package_directory));
    for (fs::directory_entry const &entry: directory_range(package_directory)) {
        fs::path const &file(entry);
        if (isImageFile(file)) {
            resourceStore.images[prefix + file.stem().string()] = loadImage(file);
        }
    }
}

sf::Sprite spriteForBlock(sf::Texture const &tex, double x, double y, double size) {
    sf::Sprite sprite(tex);
    sprite.setPosition(sf::Vector2f(x,y));
    sprite.setScale(sf::Vector2f(size*1.f/tex.getSize().x, size*1.f/tex.getSize().y));
    return sprite;
}
}//namespace

LevelResources loadLevelResources(std::string const &levelPath, std::string const &globalsPath) {
    LevelResources resources;
    loadPackage(resources, "global.", globalsPath);
    loadPackage(resources, "", levelPath);
    return resources;
}


sf::Image loadAndBakeWallImage(Wall const &wall) {
    int const segmentSize = wall.segmentSize()/100;
    int const roomWidth = wall.roomWidth()/100;
    int const roomHeight = wall.roomHeight()/100;
    
    int const roomIndexWidth = roomWidth/segmentSize;
    int const roomIndexHeight = roomHeight/segmentSize;
    
    sf::RenderTexture foregroundTexture;
    foregroundTexture.create(roomWidth, roomHeight);
    foregroundTexture.clear(sf::Color(0,0,0,0));
    sf::Texture blockTextures[2][2][2][2];
    sf::Texture cornerTextures[2][2];
    
    for (int right(0); right <= 1; ++right) {
        for(int top(0); top <= 1; ++top) {
            for (int left(0); left <= 1; ++left) {
                for (int bottom(0); bottom <= 1; ++bottom) {
                    std::stringstream filename;
                    filename << "Tilesets/" << wall.tilesetName() << right << top << left << bottom << ".png";
                    blockTextures[right][top][left][bottom] = loadTexture(filename.str());
                }
            }
        }
    }
    
    for (int bottom(0); bottom <= 1; ++bottom) {
        for (int right(0); right <= 1; ++right) {
            std::stringstream filename;
            filename << "Tilesets/" << wall.tilesetName() << (bottom ? "B":"T") << (right ? "R":"L") << ".png";
            cornerTextures[bottom][right] = loadTexture(filename.str());
        }
    }
    
    for(int x(0), xend(roomIndexWidth); x != xend; ++x) {
        for(int y(0), yend(roomIndexHeight); y != yend; ++y) {
            if (wall.atIndex(x, y)) {
                foregroundTexture.draw(spriteForBlock(blockTextures[wall.atIndex(x+1, y)][wall.atIndex(x, y-1)][wall.atIndex(x-1, y)][wall.atIndex(x, y+1)], x*segmentSize, y*segmentSize, segmentSize));
                foregroundTexture.draw(spriteForBlock(blockTextures[wall.atIndex(x+1, y)][wall.atIndex(x, y-1)][wall.atIndex(x-1, y)][wall.atIndex(x, y+1)], x*segmentSize, y*segmentSize, segmentSize));
                for (int vpos(-1); vpos <= 1; vpos+=2) {
                    for(int hpos(-1); hpos <= 1; hpos+=2) {
                        if(wall.atIndex(x+hpos, y) && wall.atIndex(x, y+vpos) && !wall.atIndex(x+hpos, y+vpos)) {
                            int const bottom((vpos+1)/2);
                            int const right((hpos+1)/2);
                            foregroundTexture.draw(spriteForBlock(cornerTextures[bottom][right], x*segmentSize+right*segmentSize/2., y*segmentSize+bottom*segmentSize/2., segmentSize/2.));
                        }
                    }
                }
            }
        }
    }
    sf::Image foregroundImage(foregroundTexture.getTexture().copyToImage());
    foregroundImage.flipVertically();
    return foregroundImage;
}

} //namespace hg
