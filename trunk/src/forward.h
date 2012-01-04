#ifndef HG_FORWARD_H
#define HG_FORWARD_H
#include <boost/type_traits/remove_reference.hpp>
namespace hg {
    template <class T> T&& forward(typename boost::remove_reference<T>::type& t) {
        return static_cast<T&&>(t);
    }
    template <class T> T&& forward(typename boost::remove_reference<T>::type&& t) {
        return static_cast<T&&>(t);
    }
}
#endif //HG_FORWARD_H
