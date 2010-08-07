#include "ObjectList.h"

#include <vector>
#include <algorithm>
using namespace std;
using namespace hg;
void ObjectList::add(const ObjectList& other, int relativeGuyIgnoreIndex)
{	
	for (unsigned int i = 0; i < other.guyList.size(); ++i)
	{
		if ((relativeGuyIgnoreIndex == -1) || (other.guyList[i].getRelativeIndex() < relativeGuyIgnoreIndex))
		{
			guyList.push_back(other.guyList[i]);
		}
	}

	for (unsigned int i = 0; i < other.boxList.size(); ++i)
	{
		boxList.push_back(other.boxList[i]);
	}
}	

bool ObjectList::operator==(const hg::ObjectList& other) const
{
    return equals(other);
}

bool ObjectList::operator!=(const hg::ObjectList& other) const
{
    return !equals(other);
}

bool ObjectList::equals(const ObjectList& other) const
{
	if (guyList.size() != other.guyList.size() || boxList.size() != other.boxList.size() )
	{
		return false;
	}
    
	for (unsigned int i = 0; i < guyList.size(); ++i)
	{
		if (!(guyList[i].equals(other.guyList[i])))
		{
			return false;
		}
	}

	for (unsigned int i = 0; i < boxList.size(); ++i)
	{
		if (!(boxList[i].equals(other.boxList[i])))
		{
			return false;
		}
	}
	return true;
}

bool ObjectList::isEmpty()
{
	return (guyList.size() == 0 && boxList.size() == 0);
}


void ObjectList::sortElements()
{
	std::sort(guyList.begin(), guyList.end());
	std::sort(boxList.begin(), boxList.end());
}

// Single Element addition
void ObjectList::addGuy(int x, int y, int xspeed, int yspeed,
                        int width, int height, hg::TimeDirection timeDirection,
                        bool boxCarrying, int boxCarrySize, hg::TimeDirection boxCarryDirection,
                        int relativeIndex, int subimage)
{
	guyList.push_back(Guy(x, y, xspeed, yspeed, 
                        width, height, 
                        timeDirection,
                        boxCarrying, boxCarrySize, boxCarryDirection, 
                        relativeIndex, subimage));
}

void ObjectList::addGuy(const Guy& toCopy)
{
	guyList.push_back(toCopy);
}

void ObjectList::addBox(int x, int y, int xspeed, int yspeed, int size, hg::TimeDirection timeDirection)
{
	boxList.push_back(Box(x, y, xspeed, yspeed, size, timeDirection));
}

void ObjectList::addBox(const Box& toCopy)
{
	boxList.push_back(toCopy);
}
