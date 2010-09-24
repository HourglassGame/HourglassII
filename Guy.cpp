#include "Guy.h"

namespace hg{
Guy::Guy(int nX,
         int nY,
         int nXspeed,
         int nYspeed,
         int nWidth,
         int nHeight,
         bool nSupported,
         bool nBoxCarrying,
         int nBoxCarrySize,
         TimeDirection nBoxCarryDirection,
         TimeDirection nTimeDirection,
         size_t nRelativeIndex,
         int nSubimage) :
referenceCount(new int(1)),
data(new Data(nX,nY,nXspeed,nYspeed,nWidth,nHeight, nSupported,
              nBoxCarrying,nBoxCarrySize,nBoxCarryDirection,
              nTimeDirection,nRelativeIndex,nSubimage))
{
}
Guy::Guy(const Guy& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

Guy::~Guy()
{
    decrementCount();
}

void Guy::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

Guy& Guy::operator=(const Guy& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

bool Guy::operator!=(const Guy& other) const
{
    return !(*this==other);
}

bool Guy::operator==(const Guy& other) const
{
	return data == other.data
        || ((data->relativeIndex == other.data->relativeIndex)
         && (data->x == other.data->x)
         && (data->y == other.data->y)
         && (data->xspeed == other.data->xspeed)
         && (data->yspeed == other.data->yspeed)
         && (data->width == other.data->width)
         && (data->height == other.data->height)
         && (data->supported == other.data->supported)
         && (data->boxCarrying == other.data->boxCarrying)
         && (data->timeDirection == other.data->timeDirection)
         && (data->subimage == other.data->subimage)
         && (data->boxCarryDirection == other.data->boxCarryDirection)
         && (data->boxCarrySize == other.data->boxCarrySize));
}

bool Guy::operator<(const Guy& other) const
{
    return data->relativeIndex < other.data->relativeIndex;
}
}//namespace hg
