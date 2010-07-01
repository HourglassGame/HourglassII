#include "InputList.h"

InputList::InputList(bool nLeft, bool nRight, bool nUp, bool nDown, bool nSpace, bool nMouseLeft, bool nMouseRight, int nMouseX, int nMouseY)
{
    left = nLeft;
	right = nRight;
	up = nUp;
	down = nDown;
	space = nSpace;
	mouseLeft = nMouseLeft;
	mouseRight = nMouseRight;
	mouseX = nMouseX;
	mouseY = nMouseY;
}

bool InputList::getLeft()
{
	return left;
}

bool InputList::getRight()
{
	return right;
}

bool InputList::getUp()
{
	return up;
}

bool InputList::getDown()
{
	return down;
}

bool InputList::getSpace()
{
	return space;
}

bool InputList::getMouseLeft()
{
	return mouseLeft;
}

bool InputList::getMouseRight()
{
	return mouseRight;
}

int InputList::getMouseX()
{
	return mouseX;
}

int InputList::getMouseY()
{
	return mouseY;
}
