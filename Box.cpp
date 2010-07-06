#include "Box.h"

#include <iostream>

Box::Box(int nX, int nY, int nXspeed, int nYspeed, int nSize, int nTimeDirection)
{
	x = nX;
	y = nY;
	xspeed = nXspeed;
	yspeed = nYspeed;
	size = nSize;
	timeDirection = nTimeDirection;
}

bool Box::equals(boost::shared_ptr<Box> other)
{
	if ( (x == other->getX()) && (y == other->getY()) && (xspeed == other->getXspeed()) && (yspeed == other->getYspeed()) && (timeDirection == other->getTimeDirection()) && (size == other->getSize()) )
	{
		return true;
	}
	return false;

}

bool Box::lessThan(boost::shared_ptr<Box> first, boost::shared_ptr<Box> second)
{
	if (first -> getX() == second -> getX())
	{
		if (first -> getY() == second -> getY())
		{
			if (first -> getXspeed() == second -> getXspeed())
			{
				if (first -> getYspeed() == second -> getYspeed())
				{
					if (first -> getTimeDirection() == second -> getTimeDirection())
					{
						if (first -> getSize() == second -> getSize())
						{
							return false;
						}
						else
						{
							return (first -> getSize() == second -> getSize());
						}
					}
					else
					{
						return (first -> getTimeDirection() < second -> getTimeDirection());
					}
				}
				else
				{
					return (first -> getYspeed() < second -> getYspeed());
				}
			}
			else
			{
				return (first -> getXspeed() < second -> getXspeed());
			}
		}
		else
		{
			return (first -> getY() < second -> getY());
		}
	}
	else
	{
		return (first -> getX() < second -> getX());
	}
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

int Box::getSize()
{
	return size;
}

int Box::getTimeDirection()
{
	return timeDirection;
}