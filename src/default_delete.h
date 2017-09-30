#ifndef HG_DEFAULT_DELETE_H
#define HG_DEFAULT_DELETE_H
#include <boost/checked_delete.hpp>
namespace hg {
    template <typename T>
    struct default_delete final {
        void operator()(T *ptr) const { boost::checked_delete(ptr); }
    };
    template <typename T>
    struct default_delete<T[]> final {
        void operator()(T ptr[]) const { boost::checked_array_delete(ptr); }
    };
}
#endif //HG_DEFAULT_DELETE_H
