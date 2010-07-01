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