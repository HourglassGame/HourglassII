#ifndef HG_DEFAULT_CLONE_H
#define HG_DEFAULT_CLONE_H
#include <boost/checked_delete.hpp>
#include <type_traits>
namespace hg {
    template<typename T>
    struct default_clone {
        default_clone() noexcept = default;
        template<typename U, typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
        default_clone(default_clone<U> const&)
        {
        }
        T *new_clone(T const &toClone) const { return toClone.clone(); }
        void delete_clone(T *toDelete) const { boost::checked_delete(toDelete); }
    };
}
#endif //HG_DEFAULT_CLONE_H
