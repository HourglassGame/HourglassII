#include "Guy.h"

namespace hg{
Guy::Guy(int nX,
         int nY,
         int nXspeed,
         int nYspeed,
         int nWidth,
         int nHeight,
         int nrelativeToPortal,
         bool nSupported,
         bool nBoxCarrying,
         int nBoxCarrySize,
         TimeDirection nBoxCarryDirection,
         int nBoxPauseLevel,
         TimeDirection nTimeDirection,
         int nPauseLevel,
         int nRelativeIndex,
         int nSubimage) :
referenceCount(new int(1)),
data(new Data(nX,nY,nXspeed,nYspeed,nWidth,nHeight, nrelativeToPortal, nSupported,
              nBoxCarrying,nBoxCarrySize,nBoxCarryDirection, nBoxPauseLevel,
              nTimeDirection,nPauseLevel,nRelativeIndex,nSubimage))
{
}

Guy::Guy(const Guy& other, TimeDirection nTimeDirection, int nPauseLevel) :
referenceCount(new int(1)),
data(new Data(other.getX(),other.getY(),other.getXspeed(),other.getYspeed(),other.getWidth(),other.getHeight(), other.getRelativeToPortal(), other.getSupported(),
              other.getBoxCarrying(),other.getBoxCarrySize(),other.getBoxCarryDirection(), other.getBoxPauseLevel(),
              nTimeDirection,nPauseLevel,other.getRelativeIndex(),other.getSubimage()))
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
            && (data->relativeToPortal == other.data->relativeToPortal)
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
