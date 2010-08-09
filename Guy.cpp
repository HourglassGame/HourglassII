#include <cassert>
#include "Guy.h"

using namespace hg;

Guy::Guy(int nX, 
         int nY, 
         int nXspeed, 
         int nYspeed, 
         int nWidth, 
         int nHeight,
         bool nBoxCarrying, 
         int nBoxCarrySize,
         TimeDirection nBoxCarryDirection,
         TimeDirection nTimeDirection, 
         int nRelativeIndex,
         int nSubimage) :
data(new Data(nX,nY,nXspeed,nYspeed,nWidth,nHeight,
              nBoxCarrying,nBoxCarrySize,nBoxCarryDirection,
              nTimeDirection,nRelativeIndex,nSubimage))
{
}

bool Guy::operator!=(const Guy& other) const
{
    return !(*this==other);
}

bool Guy::operator==(const Guy& other) const
{
	return (data->relativeIndex == other.data->relativeIndex) 
        && (data->x == other.data->x) 
        && (data->y == other.data->y) 
        && (data->xspeed == other.data->xspeed) 
        && (data->yspeed == other.data->yspeed) 
        && (data->width == other.data->width) 
        && (data->height == other.data->height) 
        && (data->boxCarrying == other.data->boxCarrying) 
        && (data->timeDirection == other.data->timeDirection) 
        && (data->subimage == other.data->subimage)
        && (data->boxCarryDirection == other.data->boxCarryDirection) 
        && (data->boxCarrySize == other.data->boxCarrySize);
}

bool Guy::operator<(const Guy& second) const
{
	assert(data->relativeIndex != second.data->relativeIndex && "Attempted to '<' 2 guys with equal relativeIndex");
    return (data->relativeIndex < second.data->relativeIndex);
}
