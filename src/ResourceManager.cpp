#include "ResourceManager.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "multi_array.h"
#include <sstream>
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

inline bool isSoundFile(fs::path const &file) {
    return file.extension() == ".wav";
}

inline sf::Image loadImage(fs::path const &file) {
    sf::Image img;
    bool loaded(img.loadFromFile(file.string()));
    assert(loaded);
    return img;
}

inline sf::SoundBuffer loadSoundBuffer(fs::path const &file) {
    sf::SoundBuffer sound;
    bool loaded(sound.loadFromFile(file.string()));
    assert(loaded);
    return sound;
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
        if (isSoundFile(file)) {
            resourceStore.sounds[prefix + file.stem().string()] = loadSoundBuffer(file);
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
    
   auto const blockImages = [&wall,segmentSize] {
        multi_array<sf::Image, 2, 2, 2, 2> blockImages;
        for (int right(0); right <= 1; ++right) {
            for (int top(0); top <= 1; ++top) {
                for (int left(0); left <= 1; ++left) {
                    for (int bottom(0); bottom <= 1; ++bottom) {
                        std::stringstream filename;
                        filename << "Tilesets/" << wall.tilesetName() << right << top << left << bottom << ".png";
                        blockImages[right][top][left][bottom] = loadImage(filename.str());
                        assert((blockImages[right][top][left][bottom].getSize() == sf::Vector2u(segmentSize,segmentSize)));
                    }
                }
            }
        }
        return blockImages;
    }();

    auto const cornerImages = [&wall,segmentSize] {
        multi_array<sf::Image, 2, 2> cornerImages;
        for (int bottom(0); bottom <= 1; ++bottom) {
            for (int right(0); right <= 1; ++right) {
                std::stringstream filename;
                filename << "Tilesets/" << wall.tilesetName() << (bottom ? "B":"T") << (right ? "R":"L") << ".png";
                cornerImages[bottom][right] = loadImage(filename.str());
                assert((cornerImages[bottom][right].getSize() == sf::Vector2u(segmentSize/2,segmentSize/2)));
            }
        }
        return cornerImages;
    }();
    
    //This should be done using a RenderTexture, but RenderTexture is
    //buggy on some platforms (in particular, the MASS3 smartboard).
    //The current implementation requires that the tileset be the same size in pixels as the level's
    //segment size (divided by 100).
    sf::Image foregroundImageBuf;
    foregroundImageBuf.create(roomWidth, roomHeight, sf::Color(0,0,0,0));
    
    for (int x(0), xend(roomIndexWidth); x != xend; ++x) {
        for (int y(0), yend(roomIndexHeight); y != yend; ++y) {
            if (wall.atIndex(x, y)) {

                foregroundImageBuf.copy(
                    blockImages
                        [wall.atIndex(x+1, y)]
                        [wall.atIndex(x, y-1)]
                        [wall.atIndex(x-1, y)]
                        [wall.atIndex(x, y+1)],
                        x*segmentSize,
                        y*segmentSize);

                for (int vpos(-1); vpos <= 1; vpos += 2) {
                    for (int hpos(-1); hpos <= 1; hpos += 2) {
                        if (wall.atIndex(x+hpos, y)
                         && wall.atIndex(x, y+vpos)
                        && !wall.atIndex(x+hpos, y+vpos))
                        {
                            int const bottom((vpos+1)/2);
                            int const right((hpos+1)/2);
                            foregroundImageBuf.copy(
                                cornerImages[bottom][right],
                                x*segmentSize + right*segmentSize/2.,
                                y*segmentSize + bottom*segmentSize/2.);
                        }
                    }
                }
            }
        }
    }
    
    return foregroundImageBuf;
}

} //namespace hg
