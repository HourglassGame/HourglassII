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

bool Box::equals(const Box& other) const
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

bool Box::lessThan(boost::shared_ptr<const Box> first, boost::shared_ptr<const Box> second)
{
	if (first -> x == second -> x)
	{
		if (first -> y == second -> y)
		{
			if (first -> xspeed == second -> xspeed)
			{
				if (first -> yspeed == second -> yspeed)
				{
					if (first -> timeDirection == second -> timeDirection)
					{
						if (first -> size == second -> size)
						{
							return false;
						}
						else
						{
							return (first -> size < second -> size);
						}
					}
					else
					{
						return (first -> timeDirection < second -> timeDirection);
					}
				}
				else
				{
					return (first -> yspeed < second -> yspeed);
				}
			}
			else
			{
				return (first -> xspeed < second -> xspeed);
			}
		}
		else
		{
			return (first -> y < second -> y);
		}
	}
	else
	{
		return (first -> x < second -> x);
	}
}
