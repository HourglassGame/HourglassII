#include "Platform.h"

#include <cassert>
#include <iostream>
using namespace ::hg;

Platform::Platform(int nX,
         int nY,
         int nXspeed,
         int nYspeed,
         int nWidth,
         int nHeight,
         int nIndex,
         TimeDirection nTimeDirection) :
referenceCount(new int(1)),
data(new Data(nX,nY,nXspeed,nYspeed,nWidth,nHeight, nIndex,nTimeDirection))
{
}

Platform::Platform(const Platform& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

Platform::~Platform()
{
    decrementCount();
}

void Platform::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

Platform& Platform::operator=(const Platform& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

bool Platform::operator!=(const Platform& other) const
{
    return !(*this==other);
}

bool Platform::operator==(const Platform& other) const
{
	return data->index == other.data->index;
}

bool Platform::operator<(const Platform& other) const
{
    return data->index < other.data->index;
}
