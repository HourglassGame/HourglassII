#include "Box.h"

Box::Box(int nX, int nY, int nXspeed, int nYspeed, int nTimeDirection)
{
	x = nX;
	y = nY;
	xspeed = nXspeed;
	yspeed = nYspeed;
	timeDirection = nTimeDirection;

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
