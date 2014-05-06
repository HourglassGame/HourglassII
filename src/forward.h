#ifndef HG_FORWARD_H
#define HG_FORWARD_H
#include <boost/type_traits/remove_reference.hpp>
#include <boost/config.hpp>
namespace hg {
#ifndef BOOST_NO_RVALUE_REFERENCES
    template <class T> constexpr T &&forward(typename boost::remove_reference<T>::type &t) noexcept {
        return static_cast<T &&>(t);
    }
    template <class T> constexpr T &&forward(typename boost::remove_reference<T>::type &&t) noexcept {
        static_assert(!std::is_lvalue_reference<T>::value,"forward called with l-value type but r-value object");
        return static_cast<T &&>(t);
    }
#endif
}
#endif //HG_FORWARD_H
