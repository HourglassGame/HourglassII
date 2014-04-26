#ifndef HG_UNIQUE_PTR_H
#define HG_UNIQUE_PTR_H
#include <memory>
#include <utility>
namespace hg {
    using std::unique_ptr;
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args &&...args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif //HG_UNIQUE_PTR_H
