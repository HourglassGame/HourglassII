#ifndef HG_FORWARD_H
#define HG_FORWARD_H
#include <boost/type_traits/remove_reference.hpp>
#include <boost/config.hpp>
namespace hg {
#ifndef BOOST_NO_RVALUE_REFERENCES
    template <class T> T&& forward(typename boost::remove_reference<T>::type& t) {
        return static_cast<T&&>(t);
    }
    template <class T> T&& forward(typename boost::remove_reference<T>::type&& t) {
        return static_cast<T&&>(t);
    }
#endif
}
#endif //HG_FORWARD_H
