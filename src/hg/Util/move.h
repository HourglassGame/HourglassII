#ifndef HG_MOVE_H
#define HG_MOVE_H
#include <boost/type_traits/remove_reference.hpp>
namespace hg{
    template <class T> typename boost::remove_reference<T>::type &&
    move(T &&t) {
        return static_cast<typename boost::remove_reference<T>::type &&>(t);
    }
}
#endif //HG_MOVE_H
