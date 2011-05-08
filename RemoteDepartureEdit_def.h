#ifndef HG_REMOTE_DEPARTURE_EDIT_DEF
#define HG_REMOTE_DEPARTURE_EDIT_DEF
#include "RemoteDepartureEdit.h"
namespace hg {
template <typename Type>
RemoteDepartureEdit<Type>::RemoteDepartureEdit(
    const PauseInitiatorID& origin,
    const Type& departure,
    bool propIntoNormal) :
        origin_(origin),
        departure_(departure),
        propIntoNormal_(propIntoNormal)
{
}

template <typename Type>
RemoteDepartureEdit<Type>::RemoteDepartureEdit(const RemoteDepartureEdit<Type>& other) :
        origin_(other.origin_),
        departure_(other.departure_),
        propIntoNormal_(other.propIntoNormal_)
{
}

template <typename Type>
RemoteDepartureEdit<Type>& RemoteDepartureEdit<Type>::operator=(const RemoteDepartureEdit<Type>& other)
{
    origin_ = other.origin_;
    departure_ = other.departure_;
    propIntoNormal_ = other.propIntoNormal_;
    return *this;
}

template <typename Type>
const Type& RemoteDepartureEdit<Type>::getDeparture() const {
    return departure_;
}

template <typename Type>
const PauseInitiatorID& RemoteDepartureEdit<Type>::getOrigin() const {
    return origin_;
}

template <typename Type>
const bool RemoteDepartureEdit<Type>::getPropIntoNormal() const {
    return propIntoNormal_;
}
    
template <typename Type>
bool RemoteDepartureEdit<Type>::operator!=(const RemoteDepartureEdit<Type>& other) const
{
    return !(*this==other);
}

template <typename Type>
bool RemoteDepartureEdit<Type>::operator==(const RemoteDepartureEdit<Type>& other) const
{
    return (origin_ == other.origin_)
           && (departure_ == other.departure_)
           && (propIntoNormal_ == other.propIntoNormal_);
}

template <typename Type>
bool RemoteDepartureEdit<Type>::operator<(const RemoteDepartureEdit<Type>& other) const
{
    if (origin_ == other.origin_) {
        if (departure_ == other.departure_) {
            if (propIntoNormal_ == other.propIntoNormal_) {
                return false;
            }
            else {
                return propIntoNormal_ < other.propIntoNormal_;
            }
        }
        else {
            return departure_ < other.departure_;
        }
    }
    else {
        return origin_ < other.origin_;
    }
}
}

#endif //HG_REMOTE_DEPARTURE_EDIT_DEF
