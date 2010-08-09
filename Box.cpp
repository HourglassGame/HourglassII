#include "Box.h"

#include <iostream>

using namespace hg;

Box::Box(int nX, int nY, int nXspeed, int nYspeed, int nSize, hg::TimeDirection nTimeDirection) :
data(new Data(nX, nY, nXspeed, nYspeed, nSize, nTimeDirection))
{
}

bool Box::operator!=(const Box& other) const
{
<<<<<<< .mine
    return !(*this==other);
}

bool Box::operator==(const Box& other) const
{
	if ( (data->x == other.data->x)
        && (data->y == other.data->y)
        && (data->xspeed == other.data->xspeed) 
        && (data->yspeed == other.data->yspeed)
        && (data->timeDirection == other.data->timeDirection) 
        && (data->size == other.data->size) )
=======
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
>>>>>>> .r93
	{
		return true;
	}
	return false;
}

bool Box::operator<(const Box& second) const
{
	if (data->x == second.data->x)
	{
		if (data->y == second.data->y)
		{
			if (data->xspeed == second.data->xspeed)
			{
				if (data->yspeed == second.data->yspeed)
				{
					if (data->timeDirection == second.data->timeDirection)
					{
						if (data->size == second.data->size)
						{
							return false;
						}
						else
						{
							return (data->size < second.data->size);
						}
					}
					else
					{
						return (data->timeDirection < second.data->timeDirection);
					}
				}
				else
				{
					return (data->yspeed < second.data->yspeed);
				}
			}
			else
			{
				return (data->xspeed < second.data->xspeed);
			}
		}
		else
		{
			return (data->y < second.data->y);
		}
	}
	else
	{
		return (data->x < second.data->x);
	}
}
