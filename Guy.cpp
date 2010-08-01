#include <stdexcept>
#include "Guy.h"

using namespace hg;

Guy::Guy(int nX, 
         int nY, 
         int nXspeed, 
         int nYspeed, 
         int nWidth, 
         int nHeight, 
         hg::TimeDirection nTimeDirection, 
         bool nBoxCarrying, 
         int nBoxCarrySize,
         hg::TimeDirection nBoxCarryDirection,
         int nRelativeIndex,
         int nSubimage) :
x(nX),
y(nY),
xspeed(nXspeed),
yspeed(nYspeed),
width(nWidth),
height(nHeight),
timeDirection(nTimeDirection),
boxCarrying(nBoxCarrying),
boxCarrySize(nBoxCarrySize),
boxCarryDirection(nBoxCarryDirection),
relativeIndex(nRelativeIndex),
subimage(nSubimage)
{
}

bool Guy::equals(const Guy& other) const
{
	if ( (x == other.x) && (y == other.y) && (xspeed == other.xspeed) && 
		(yspeed == other.yspeed) && (width == other.width) && (height == other.height) &&
		(boxCarrying == other.boxCarrying) && (timeDirection == other.timeDirection) && 
        (relativeIndex == other.relativeIndex) && 
		(subimage == other.subimage) && 
        (boxCarryDirection == other.boxCarryDirection) && 
        (boxCarrySize == other.boxCarrySize))
	{
		return true;
	}
	return false;
}

bool Guy::lessThan(boost::shared_ptr<Guy> first , boost::shared_ptr<Guy> second)
{
	if (first -> relativeIndex == second -> relativeIndex)
	{
		std::runtime_error( "Attempted to '<' 2 guys with equal relativeIndex");
		return false;
	}
	else
	{
		return (first -> relativeIndex < second -> relativeIndex);
	}
}
