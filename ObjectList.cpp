#include "ObjectList.h"

#include <vector>
using namespace std;

ObjectList::ObjectList()
{
	
}

void ObjectList::addGuy(int x, int y, int xspeed, int yspeed, int timeDirection, bool boxCarrying, int relativeIndex)
{
	guyList.push_back(Guy(x, y, xspeed, yspeed, timeDirection, boxCarrying, relativeIndex));
}

void ObjectList::addBox(int x, int y, int xspeed, int yspeed, int timeDirection)
{
	boxList.push_back(Box(x, y, xspeed, yspeed, timeDirection));
}

void ObjectList::addItem(int x, int y, int xspeed, int yspeed, int timeDirection, int type)
{
	itemList.push_back(Item(x, y, xspeed, yspeed, timeDirection, type));
}

void ObjectList::addPickup(int x, int y, int platformAttachment, int timeDirection, int type)
{
	pickupList.push_back(Pickup(x, y, platformAttachment, timeDirection, type));
}

void ObjectList::addPlatform(int x, int y, int xspeed, int yspeed, int timeDirection, int id)
{
	platformList.push_back(Platform(x, y, xspeed, yspeed, timeDirection, id));
}

void ObjectList::addSwitch(int x, int y, int type, int platformAttachment, int id)
{
	switchList.push_back(Switch(x, y, type, platformAttachment, id));
}


vector<Guy> ObjectList::getGuyList()
{
	return guyList;
}

vector<Box> ObjectList::getBoxList()
{
	return boxList;
}

vector<Item> ObjectList::getItemList()
{
	return itemList;
}

vector<Pickup> ObjectList::getPickupList()
{
	return pickupList;
}

vector<Platform> ObjectList::getPlatformList()
{
	return platformList;
}

vector<Switch> ObjectList::getSwitchList()
{
	return switchList;
}
