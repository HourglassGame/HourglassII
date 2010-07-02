#include "Guy.h"

Guy::Guy(int nX, int nY, int nXspeed, int nYspeed, int nTimeDirection, bool nBoxCarrying, int nRelativeIndex)
{
	x = nX;
	y = nY;
	xspeed = nXspeed;
	yspeed = nYspeed;
	timeDirection = nTimeDirection;
	boxCarrying = nBoxCarrying;
	relativeIndex = nRelativeIndex;

}

bool Guy::operator==(Guy* other)
{
	if ( (x == other->getX()) && (y == other->getY()) && (xspeed == other->getXspeed()) && (yspeed == other->getYspeed()) & 
		(boxCarrying == other->getBoxCarrying()) && (timeDirection == other->getTimeDirection()) && (relativeIndex == other -> getRelativeIndex()) )
	{
		return true;
	}
	return false;
}

bool Guy::operator!=(Guy* other)
{
	return !(*this == other);
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

bool Guy::getBoxCarrying()
{
	return boxCarrying;
}

int Guy::getTimeDirection()
{
	return timeDirection;
}

int Guy::getRelativeIndex()
{
	return relativeIndex;
}