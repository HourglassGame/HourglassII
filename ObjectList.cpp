#include "ObjectList.h"

#include <vector>
using namespace std;

ObjectList::ObjectList()
{
	
}

void ObjectList::add(const ObjectList& other, int relativeGuyIgnoreIndex)
{
	
	for (unsigned int i = 0; i < other.getGuyList().size(); ++i)
	{
		if ((relativeGuyIgnoreIndex == -1) || (other.getGuyList()[i]->getRelativeIndex() < relativeGuyIgnoreIndex))
		{
			addGuy(other.getGuyList()[i]);
		}
	}

	for (unsigned int i = 0; i < other.getBoxList().size(); ++i)
	{
		addBox(other.getBoxList()[i]);
	}
}	

bool ObjectList::equals(const ObjectList& other) const
{

	if (guyList.size() != other.getGuyList().size() || boxList.size() != other.getBoxList().size() )
	{
		return false;
	}

	for (unsigned int i = 0; i < guyList.size(); ++i)
	{
		if (!(guyList[i]->equals(other.getGuyList()[i])))
		{
			return false;
		}
	}

	for (unsigned int i = 0; i < boxList.size(); ++i)
	{
		if (!(boxList[i]->equals(other.getBoxList()[i])))
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
	std::sort(guyList.begin(), guyList.end(), Guy::lessThan);
	std::sort(boxList.begin(), boxList.end(), Box::lessThan);
}

// Single Element addition
void ObjectList::addGuy(int x, int y, int xspeed, int yspeed, int width, int height, int timeDirection, bool boxCarrying, int boxCarrySize, int boxCarryDirection, int relativeIndex, int subimage)
{
	
	guyList.push_back(boost::shared_ptr<Guy> (new Guy(x, y, xspeed, yspeed, width, height, timeDirection, boxCarrying, boxCarrySize, boxCarryDirection, relativeIndex, subimage)));
}

void ObjectList::addGuy(boost::shared_ptr<Guy> toCopy)
{
	guyList.push_back(toCopy);
}

void ObjectList::addBox(int x, int y, int xspeed, int yspeed, int size, int timeDirection)
{
	boxList.push_back(boost::shared_ptr<Box> (new Box(x, y, xspeed, yspeed, size, timeDirection)));
}

void ObjectList::addBox(boost::shared_ptr<Box> toCopy)
{
	boxList.push_back(toCopy);
}

void ObjectList::addItem(int x, int y, int xspeed, int yspeed, int timeDirection, int type)
{
	itemList.push_back(boost::shared_ptr<Item> (new Item(x, y, xspeed, yspeed, timeDirection, type)));
}

void ObjectList::addPickup(int x, int y, int platformAttachment, int timeDirection, int type)
{
	pickupList.push_back(boost::shared_ptr<Pickup> (new Pickup(x, y, platformAttachment, timeDirection, type)));
}

void ObjectList::addPlatform(int x, int y, int xspeed, int yspeed, int timeDirection, int id)
{
	platformList.push_back(boost::shared_ptr<Platform> (new Platform(x, y, xspeed, yspeed, timeDirection, id)));
}

void ObjectList::addSwitch(int x, int y, int type, int platformAttachment, int id)
{
	switchList.push_back(boost::shared_ptr<Switch> (new Switch(x, y, type, platformAttachment, id)));
}

// Getters
vector<boost::shared_ptr<Guy> > ObjectList::getGuyList() const
{
	return guyList;
}

vector<boost::shared_ptr<Box> > ObjectList::getBoxList() const
{
	return boxList;
}

vector<boost::shared_ptr<Item> > ObjectList::getItemList() const
{
	return itemList;
}

vector<boost::shared_ptr<Pickup> > ObjectList::getPickupList() const
{
	return pickupList;
}

vector<boost::shared_ptr<Platform> > ObjectList::getPlatformList() const
{
	return platformList;
}

vector<boost::shared_ptr<Switch> > ObjectList::getSwitchList() const
{
	return switchList;
}
