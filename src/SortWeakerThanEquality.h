#ifndef HG_SORTED_BY_INDEX_H
#define HG_SORTED_BY_INDEX_H

namespace hg {
template<typename T>
struct sort_weaker_than_equality final
{
    static bool const value = false;
};
}

#endif //HG_SORTED_BY_INDEX_H
