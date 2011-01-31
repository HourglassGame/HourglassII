#ifndef HG_DEPARTURE_THIEF_CPP
#define HG_DEPARTURE_THIEF_CPP
#include "RemoteDepartureEdit.h"

namespace hg {
template <class Type> RemoteDepartureEdit<Type>::RemoteDepartureEdit(PauseInitiatorID origin, Type departure, bool propIntoNormal) :
origin_(origin),
departure_(departure),
propIntoNormal_(propIntoNormal)
{
}

template <class Type> RemoteDepartureEdit<Type>::RemoteDepartureEdit(const RemoteDepartureEdit<Type>& other) :
origin_(other.origin_),
departure_(other.departure_),
propIntoNormal_(other.propIntoNormal_)
{
}

template <class Type> RemoteDepartureEdit<Type>::~RemoteDepartureEdit()
{
}

template <class Type> RemoteDepartureEdit<Type>& RemoteDepartureEdit<Type>::operator=(const RemoteDepartureEdit<Type>& other)
{
    origin_ = other.origin_;
    departure_ = other.departure_;
    propIntoNormal_ = other.propIntoNormal_;
    return *this;
}

template <class Type> bool RemoteDepartureEdit<Type>::operator!=(const RemoteDepartureEdit<Type>& other) const
{
    return !(*this==other);
}

template <class Type> bool RemoteDepartureEdit<Type>::operator==(const RemoteDepartureEdit<Type>& other) const
{
	return (origin_ == other.origin_)
            && (departure_ == other.departure_)
            && (propIntoNormal_ == other.propIntoNormal_);
}

template <class Type> bool RemoteDepartureEdit<Type>::operator<(const RemoteDepartureEdit<Type>& other) const
{
    if (origin_ == other.origin_)
    {
        if (departure_ == other.departure_)
        {
            if (propIntoNormal_ == other.propIntoNormal_)
            {
                return false;
            }
            else
            {
                return propIntoNormal_ < other.propIntoNormal_;
            }
        }
        else
        {
            return departure_ < other.departure_;
        }
    }
    else
    {
        return origin_ < other.origin_;
    }
}
}
#endif //HG_DEPARTURE_THIEF_CPP
