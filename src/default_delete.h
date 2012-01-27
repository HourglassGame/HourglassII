#ifndef HG_DEFAULT_DELETE_H
#define HG_DEFAULT_DELETE_H
namespace hg {
    template <typename T>
    struct default_delete {
        void operator()(T* ptr) const { delete ptr; }
    };
    template <typename T>
    struct default_delete<T[]> {
        void operator()(T ptr[]) const { delete[] ptr; }
    };
}
#endif //HG_DEFAULT_DELETE_H
