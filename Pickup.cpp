#include "Pickup.h"
using namespace hg;
Pickup::Pickup(int nX, int nY, hg::TimeDirection nTimeDirection, int nType ,int nPlatformAttachment)
{
	x = nX;
	y = nY;
	timeDirection = nTimeDirection;
	type = nType;
	platformAttachment = nPlatformAttachment;

}

