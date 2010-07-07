#include <stdexcept>
#include "Guy.h"

int Guy::animationLength = 13;

Guy::Guy(int nX, int nY, int nXspeed, int nYspeed, int nWidth, int nHeight, int nTimeDirection, bool nBoxCarrying, int nBoxCarrySize,int nBoxCarryDirection, int nRelativeIndex, int nSubimage)
{
	x = nX;
	y = nY;
	xspeed = nXspeed;
	yspeed = nYspeed;
	width = nWidth;
	height = nHeight;
	timeDirection = nTimeDirection;
	boxCarrying = nBoxCarrying;
	boxCarrySize = nBoxCarrySize;
	boxCarryDirection = nBoxCarryDirection;
	relativeIndex = nRelativeIndex;
	subimage = nSubimage;

}

bool Guy::equals(boost::shared_ptr<Guy> other)
{
	if ( (x == other->getX()) && (y == other->getY()) && (xspeed == other->getXspeed()) && 
		(yspeed == other->getYspeed()) && (width == other->getWidth()) && (height == other->getHeight()) &&
		(boxCarrying == other->getBoxCarrying()) && (timeDirection == other->getTimeDirection()) && (relativeIndex == other -> getRelativeIndex()) && 
		(subimage == other -> getSubimage()) && (boxCarryDirection == other->getBoxCarryDirection()) && (boxCarrySize == other->getBoxCarrySize()))
	{
		return true;
	}
	return false;
}

bool Guy::lessThan(boost::shared_ptr<Guy> first , boost::shared_ptr<Guy> second)
{
	if (first -> getRelativeIndex() == second -> getRelativeIndex())
	{
		std::runtime_error( "Attempted to '<' 2 guys with equal relativeIndex");
		return false;
	}
	else
	{
		return (first -> getRelativeIndex() < second -> getRelativeIndex());
	}
}

int Guy::getX()
{
	return x;
}

int Guy::getY()
{
	return y;
}

int Guy::getXspeed()
{
	return xspeed;
}

int Guy::getYspeed()
{
	return yspeed;
}

int Guy::getWidth()
{
	return width;
}

int Guy::getHeight()
{
	return height;
}

bool Guy::getBoxCarrying()
{
	return boxCarrying;
}

int Guy::getBoxCarrySize()
{
	return boxCarrySize;
}

int Guy::getBoxCarryDirection()
{
	return boxCarryDirection;
}

int Guy::getTimeDirection()
{
	return timeDirection;
}

int Guy::getRelativeIndex()
{
	return relativeIndex;
}

int Guy::getSubimage()
{
	return subimage;
}