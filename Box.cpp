#include "Box.h"

#include <iostream>

using namespace hg;

Box::Box(int nX, int nY, int nXspeed, int nYspeed, int nSize, hg::TimeDirection nTimeDirection) :
x(nX),
y(nY),
xspeed(nXspeed),
yspeed(nYspeed),
size(nSize),
timeDirection(nTimeDirection)
{
}

bool Box::operator!=(const Box& other) const
{
    return !(*this==other);
}

bool Box::operator==(const Box& other) const
{
	if ( (x == other.x)
        && (y == other.y)
        && (xspeed == other.xspeed) 
        && (yspeed == other.yspeed)
        && (timeDirection == other.timeDirection) 
        && (size == other.size) )
	{
		return true;
	}
	return false;
}

bool Box::operator<(const Box& second) const
{
	if (x == second.x)
	{
		if (y == second.y)
		{
			if (xspeed == second . xspeed)
			{
				if (yspeed == second.yspeed)
				{
					if (timeDirection == second.timeDirection)
					{
						if (size == second.size)
						{
							return false;
						}
						else
						{
							return (size < second.size);
						}
					}
					else
					{
						return (timeDirection < second.timeDirection);
					}
				}
				else
				{
					return (yspeed < second.yspeed);
				}
			}
			else
			{
				return (xspeed < second.xspeed);
			}
		}
		else
		{
			return (y < second.y);
		}
	}
	else
	{
		return (x < second.x);
	}
}
