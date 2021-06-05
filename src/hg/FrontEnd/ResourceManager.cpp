#include "ResourceManager.h"
#include "GameDisplayHelpers.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <SFML/Graphics/Image.hpp>
#include "hg/Util/multi_array.h"
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
	static_cast<void>(loaded);
	return img;
}

inline sf::SoundBuffer loadSoundBuffer(fs::path const &file) {
	sf::SoundBuffer sound;
	bool loaded(sound.loadFromFile(file.string()));
	assert(loaded);
	static_cast<void>(loaded);
	return sound;
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

}//namespace

LevelResources loadLevelResources(std::string const &levelPath, std::string const &globalsPath) {
	LevelResources resources;
	loadPackage(resources, "global.", globalsPath);
	loadPackage(resources, "", levelPath);
	return resources;
}

} //namespace hg
