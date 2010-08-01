#include "Platform.h"
using namespace hg;
Platform::Platform(int nX, int nY, int nXspeed, int nYspeed, int nTimeDirection, int nId) :
x(nX),
y(nY),
xspeed(nXspeed),
yspeed(nYspeed),
timeDirection(nTimeDirection),
id(nId)
{
}