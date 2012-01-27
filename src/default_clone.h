#ifndef HG_DEFAULT_CLONE_H
#define HG_DEFAULT_CLONE_H
namespace hg {
    template<typename T>
    struct default_clone {
        T* new_clone(T const& toClone) const { return toClone.clone(); }
        void delete_clone(T* toDelete) const { delete toDelete; }
    };
}
#endif //HG_DEFAULT_CLONE_H
