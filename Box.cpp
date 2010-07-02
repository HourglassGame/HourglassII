#include "Box.h"

Box::Box(int nX, int nY, int nXspeed, int nYspeed, int nTimeDirection)
{
	x = nX;
	y = nY;
	xspeed = nXspeed;
	yspeed = nYspeed;
	timeDirection = nTimeDirection;

}


bool Box::operator==(Box* other)
{
	if ( (x == other->getX()) && (y == other->getY()) && (xspeed == other->getXspeed()) && (yspeed == other->getYspeed()) && (timeDirection == other->getTimeDirection()) )
	{
		return true;
	}
	return false;
}

bool Box::operator!=(Box* other)
{
	return !(*this == other);
}

int Box::getX()
{
	return x;
}

int Box::getY()
{
	return y;
}

int Box::getXspeed()
{
	return xspeed;
}

int Box::getYspeed()
{
	return yspeed;
}

int Box::getTimeDirection()
{
	return timeDirection;
}

