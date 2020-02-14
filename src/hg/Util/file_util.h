#ifndef HG_FILE_UTIL_H
#define HG_FILE_UTIL_H
#include <boost/filesystem.hpp>
#include <sstream>
#include <vector>
namespace hg {
inline std::vector<char> loadFileIntoVector(
    boost::filesystem::path const &filename)
{
    boost::filesystem::ifstream file;
    file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
    file.open(filename, std::ifstream::in | std::ifstream::binary);

    std::stringstream ss;
    if (!(ss << file.rdbuf())) {
        throw std::exception("Couldn't read file");
    }
    auto const s{ss.str()};
    return std::vector<char>(s.begin(), s.end());
}
}
#endif // !HG_FILE_UTIL_H
