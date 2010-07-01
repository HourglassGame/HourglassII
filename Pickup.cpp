#include "Pickup.h"

Pickup::Pickup(int nX, int nY, int nPlatformAttachment, int nTimeDirection, int nType)
{
	x = nX;
	y = nY;
	timeDirection = nTimeDirection;
	type = nType;
	platformAttachment = nPlatformAttachment;

}