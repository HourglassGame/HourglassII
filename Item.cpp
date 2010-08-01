#include "Item.h"
using namespace hg;
Item::Item(int nX, int nY, int nXspeed, int nYspeed, int nTimeDirection, int nType) :
x(nX),
y(nY),
xspeed(nXspeed),
yspeed(nYspeed),
timeDirection(nTimeDirection),
type(nType)
{
}
