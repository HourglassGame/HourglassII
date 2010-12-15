#include "Portal.h"

#include <cassert>
#include <iostream>

namespace hg {
Portal::Portal(int nX,
        int nY,
        int nXspeed,
        int nYspeed,
        int nWidth,
        int nHeight,
        int nIndex,
        TimeDirection nTimeDirection,
        int nPauseLevel,
        int ncharges,
        bool nactive,
        int nxDestination,
        int nyDestination,
        int ndestinationIndex,
        int ntimeDestination,
        bool nrelativeTime) :
referenceCount(new int(1)),
data(new Data(nX,nY,nXspeed,nYspeed,nWidth,nHeight, nIndex,nTimeDirection, nPauseLevel, ncharges,
              nactive, nxDestination, nyDestination, ndestinationIndex, ntimeDestination, nrelativeTime))
{
}

Portal::Portal(const Portal& other, hg::TimeDirection nTimeDirection, int nPauseLevel) :
referenceCount(new int(1)),
data(new Data(other.getX(), other.getY(), other.getXspeed(), other.getYspeed(), other.getWidth(), other.getHeight(), other.getIndex(), nTimeDirection, nPauseLevel,
              getCharges(), other.getActive(), other.getXdestination(), other.getYdestination(),
			  other.getDestinationIndex(), other.getTimeDestination(), other.getRelativeTime() )
              )
{
}

Portal::Portal(const Portal& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

Portal::~Portal()
{
    decrementCount();
}

void Portal::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

Portal& Portal::operator=(const Portal& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

bool Portal::operator!=(const Portal& other) const
{
    return !(*this==other);
}

bool Portal::operator==(const Portal& other) const
{
	return data->index == other.data->index
            && (data->x == other.data->x)
            && (data->y == other.data->y)
            && (data->xspeed == other.data->xspeed)
            && (data->yspeed == other.data->yspeed)
            && (data->width == other.data->width)
            && (data->height == other.data->height)
            && (data->pauseLevel == other.data->pauseLevel)
            && (data->charges == other.data->charges)
            && (data->active == other.data->active)
            && (data->xDestination  == other.data->xDestination)
            && (data->yDestination == other.data->yDestination)
            && (data->destinationIndex == other.data->destinationIndex)
            && (data->timeDestination == other.data->timeDestination)
            && (data->relativeTime == other.data->relativeTime)
            ;
}

bool Portal::operator<(const Portal& other) const
{
    return data->index < other.data->index;
}
}
