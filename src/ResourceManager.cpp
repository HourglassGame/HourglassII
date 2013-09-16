#include "ResourceManager.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "Foreach.h"
namespace hg {
namespace fs = boost::filesystem;
inline
boost::iterator_range<fs::directory_iterator>
directory_range(fs::path const& path)
{
    return boost::iterator_range<fs::directory_iterator>(
        fs::directory_iterator(path),
        fs::directory_iterator());
}

namespace {
inline bool isImageFile(fs::path const& file) {
    return file.extension() == ".png";
}

inline sf::Image loadImage(fs::path const& file) {
    sf::Image img;
    assert(img.LoadFromFile(file.string()));
    return img;
}

inline void loadPackage(LevelResources& resourceStore, std::string const& prefix, std::string const& directory) {
    fs::path package_directory(directory);
    assert(fs::is_directory(package_directory));
    //std::cout << "LOADING PACKAGE: " << prefix << " " << directory << "\n";
    foreach (fs::directory_entry const& entry, directory_range(package_directory)) {
        fs::path const& file(entry);
        if (isImageFile(file)) {
            resourceStore.images[prefix + file.stem().string()] = loadImage(file);
            //std::cout << prefix + file.stem().string() << " Loaded!\n";
        }
    }
}
}

LevelResources loadLevelResources(std::string const& levelPath, std::string const& globalsPath) {
    LevelResources resources;
    loadPackage(resources, "global.", globalsPath);
    loadPackage(resources, "", levelPath);
    return resources;
}

}
