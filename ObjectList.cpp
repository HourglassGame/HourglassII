#include "ObjectList.h"

#include <vector>
using namespace std;

ObjectList::ObjectList()
{
	
}

ObjectList::~ObjectList()
{
	guyList.clear();
	boxList.clear();
	itemList.clear();
	pickupList.clear();
	platformList.clear();
	switchList.clear();
}

ObjectList* ObjectList::operator+=(ObjectList* other)
{
	
	for (int i = 0; i < other->getGuyList().size(); ++i)
	{
		addGuy(other->getGuyList()[i]);
	}

	for (int i = 0; i < other->getBoxList().size(); ++i)
	{
		addBox(other->getBoxList()[i]);
	}

	return this;
}	

ObjectList* ObjectList::operator+(ObjectList* other)
{
	ObjectList* result = new ObjectList();
	result->operator += (this);
	result->operator += (other);
	return result;
}
	

bool ObjectList::operator==(ObjectList* other)
{

	if (guyList.size() != other->getGuyList().size() || boxList.size() != other->getBoxList().size() )
	{
		return false;
	}

	for (int i = 0; i < guyList.size(); ++i)
	{
		if (guyList[i]->operator != (other->getGuyList()[i]))
		{
			return false;
		}
	}

	for (int i = 0; i < boxList.size(); ++i)
	{
		if (boxList[i]->operator != (other->getBoxList()[i]))
		{
			return false;
		}
	}

	return true;
}	

bool ObjectList::operator!=(ObjectList* other)
{
	return !(*this == other);
}

void ObjectList::addGuy(int x, int y, int xspeed, int yspeed, int timeDirection, bool boxCarrying, int relativeIndex)
{
	guyList.push_back(new Guy(x, y, xspeed, yspeed, timeDirection, boxCarrying, relativeIndex));
}

void ObjectList::addGuy(Guy* toCopy)
{
	guyList.push_back(new Guy(toCopy->getX(), toCopy->getY(), toCopy->getXspeed(), toCopy->getYspeed(), 
		toCopy->getTimeDirection(), toCopy->getBoxCarrying(), toCopy->getRelativeIndex() ));
}

void ObjectList::addBox(int x, int y, int xspeed, int yspeed, int timeDirection)
{
	boxList.push_back(new Box(x, y, xspeed, yspeed, timeDirection));
}

void ObjectList::addBox(Box* toCopy)
{
	boxList.push_back(new Box(toCopy->getX(), toCopy->getY(), toCopy->getXspeed(), toCopy->getYspeed(), toCopy->getTimeDirection() ));
}

void ObjectList::addItem(int x, int y, int xspeed, int yspeed, int timeDirection, int type)
{
	itemList.push_back(new Item(x, y, xspeed, yspeed, timeDirection, type));
}

void ObjectList::addPickup(int x, int y, int platformAttachment, int timeDirection, int type)
{
	pickupList.push_back(new Pickup(x, y, platformAttachment, timeDirection, type));
}

void ObjectList::addPlatform(int x, int y, int xspeed, int yspeed, int timeDirection, int id)
{
	platformList.push_back(new Platform(x, y, xspeed, yspeed, timeDirection, id));
}

void ObjectList::addSwitch(int x, int y, int type, int platformAttachment, int id)
{
	switchList.push_back(new Switch(x, y, type, platformAttachment, id));
}


vector<Guy*> ObjectList::getGuyList()
{
	return guyList;
}

vector<Box*> ObjectList::getBoxList()
{
	return boxList;
}

vector<Item*> ObjectList::getItemList()
{
	return itemList;
}

vector<Pickup*> ObjectList::getPickupList()
{
	return pickupList;
}

vector<Platform*> ObjectList::getPlatformList()
{
	return platformList;
}

vector<Switch*> ObjectList::getSwitchList()
{
	return switchList;
}
