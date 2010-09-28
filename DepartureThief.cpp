#ifndef HG_DEPARTURE_THIEF_CPP
#define HG_DEPARTURE_THIEF_CPP
#include "DepartureThief.h"

namespace hg {
template <class Type> DepartureThief<Type>::DepartureThief(PauseInitiatorID origin, Type departure) :
referenceCount(new int(1)),
data(new Data(origin, departure))
{
}

template <class Type> DepartureThief<Type>::DepartureThief(const DepartureThief<Type>& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

template <class Type> DepartureThief<Type>::~DepartureThief()
{
    decrementCount();
}

template <class Type> void DepartureThief<Type>::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

template <class Type> DepartureThief<Type>& DepartureThief<Type>::operator=(const DepartureThief<Type>& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

template <class Type> bool DepartureThief<Type>::operator!=(const DepartureThief<Type>& other) const
{
    return !(*this==other);
}

template <class Type> bool DepartureThief<Type>::operator==(const DepartureThief<Type>& other) const
{
	return (data->origin == other.data->origin)
            && (data->departure == other.data->departure);
}

template <class Type> bool DepartureThief<Type>::operator<(const DepartureThief<Type>& other) const
{
    if (data != other.data) {
        if (data->origin == other.data->origin)
        {
            if (data->departure == other.data->departure)
            {
                return false;
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
