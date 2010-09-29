#ifndef HG_DEPARTURE_THIEF_CPP
#define HG_DEPARTURE_THIEF_CPP
#include "RemoteDepartureEdit.h"

namespace hg {
template <class Type> RemoteDepartureEdit<Type>::RemoteDepartureEdit(PauseInitiatorID origin, Type departure, bool propIntoNormal) :
referenceCount(new int(1)),
data(new Data(origin, departure, propIntoNormal))
{
}

template <class Type> RemoteDepartureEdit<Type>::RemoteDepartureEdit(const RemoteDepartureEdit<Type>& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

template <class Type> RemoteDepartureEdit<Type>::~RemoteDepartureEdit()
{
    decrementCount();
}

template <class Type> void RemoteDepartureEdit<Type>::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

template <class Type> RemoteDepartureEdit<Type>& RemoteDepartureEdit<Type>::operator=(const RemoteDepartureEdit<Type>& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

template <class Type> bool RemoteDepartureEdit<Type>::operator!=(const RemoteDepartureEdit<Type>& other) const
{
    return !(*this==other);
}

template <class Type> bool RemoteDepartureEdit<Type>::operator==(const RemoteDepartureEdit<Type>& other) const
{
	return (data->origin == other.data->origin)
            && (data->departure == other.data->departure)
            && (data->propIntoNormal == other.data->propIntoNormal);
}

template <class Type> bool RemoteDepartureEdit<Type>::operator<(const RemoteDepartureEdit<Type>& other) const
{
    if (data != other.data) {
        if (data->origin == other.data->origin)
        {
            if (data->departure == other.data->departure)
            {
                if (data->propIntoNormal == other.data->propIntoNormal)
                {
                    return false;
                }
                else
                {
                    return (data->propIntoNormal < other.data->propIntoNormal);
                }
            }
            else
            {
                return (data->departure < other.data->departure);
            }
        }
        else
        {
            return (data->origin < other.data->origin);
        }
    }
    else
    {
        return false;
    }
}
}
#endif //HG_DEPARTURE_THIEF_CPP
