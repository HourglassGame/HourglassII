#ifndef HG_VECTOR_OF_H
#define HG_VECTOR_OF_H
#include "mt/boost/container/vector.hpp"
namespace hg {
template<typename T>
struct vector_of {
    typedef mt::boost::container::vector<T> type;
};
}
#endif //HG_VECTOR_OF_H
