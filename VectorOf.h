#ifndef HG_VECTOR_OF_H
#define HG_VECTOR_OF_H
#include "mt/std/vector"
namespace hg {
template<typename T>
struct vector_of {
    typedef typename mt::std::vector<T>::type type;
};
}
#endif //HG_VECTOR_OF_H
