#ifndef HG_UTIL_H
#define HG_UTIL_H
#include <cstring>
namespace hg {
    inline auto const strcmporder{[](char const * const a, char const * const b) {return std::strcmp(a, b) <  0; }};
    inline auto const strcmpeq   {[](char const * const a, char const * const b) {return std::strcmp(a, b) == 0; }};
}
#endif //!HG_UTIL_H
