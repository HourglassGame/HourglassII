#include "InputList.h"
using namespace hg;
InputList::InputList(bool nLeft, 
                     bool nRight,
                     bool nUp, 
                     bool nDown,
                     bool nSpace,
                     bool nMouseLeft,
                     bool nMouseRight, 
                     int nMouseX, 
                     int nMouseY) :
left(nLeft),
right(nRight),
up(nUp),
down(nDown),
space(nSpace),
mouseLeft(nMouseLeft),
mouseRight(nMouseRight),
mouseX(nMouseX),
mouseY(nMouseY)
{
}
