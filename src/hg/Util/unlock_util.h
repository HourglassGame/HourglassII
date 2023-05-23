#ifndef HG_UNLOCK_UTIL_H
#define HG_UNLOCK_UTIL_H
#include <boost/filesystem.hpp>
#include <string_view>
namespace hg {
inline bool IsLevelComplete(
	std::string_view const name)
{
	return boost::filesystem::exists((boost::filesystem::path("user") /= "saves") /= name.data());
}
inline void MarkLevelAsComplete(
	std::string_view const name)
{
	auto saveDir{ boost::filesystem::path("user") /= "saves" };
	boost::filesystem::create_directories(saveDir);
	boost::filesystem::ofstream(saveDir /= name.data());
}
}
#endif // !HG_UNLOCK_UTIL_H
