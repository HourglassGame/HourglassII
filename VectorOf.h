#ifndef HG_VECTOR_OF_H
#define HG_VECTOR_OF_H
#include <vector>
namespace hg {
template<typename T>
struct vector_of {
    typedef std::vector<T> type;
};
}
#endif //HG_VECTOR_OF_H
