#ifndef HG_UNLOCK_UTIL_H
#define HG_UNLOCK_UTIL_H
#include <boost/filesystem.hpp>
namespace hg {
inline bool IsLevelUnlocked(
	std::string name)
{
	return boost::filesystem::exists(std::string("saves/") + name);
}
}
#endif // !HG_UNLOCK_UTIL_H
