#ifndef HG_AS_LVALUE_H
#define HG_AS_LVALUE_H

namespace hg {
    template<typename T>
    constexpr T &as_lvalue(T && val) noexcept {
        return val;
    }
}

#endif //HG_AS_LVALUE_H
