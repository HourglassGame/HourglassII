
#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST

#include "Guy.h"
#include "Box.h"
#include "Item.h"
#include "Pickup.h"
#include "Platform.h"
#include "Switch.h"

#endif // INC_OBJECTLIST

#include <vector>
using namespace std;

// Object list stores all data sent between frames or to rendering engine
class ObjectList  
{
public:
	ObjectList();
	~ObjectList();
	
	void addGuy(int x, int y, int xspeed, int yspeed, int timeDirection, bool boxCarrying, int relativeIndex);
	void addBox(int x, int y, int xspeed, int yspeed, int timeDirection); // I know it's the same as Item but this way is more visible
	void addItem(int x, int y, int xspeed, int yspeed, int timeDirection, int type); // jetpack, gun
	void addPickup(int x, int y, int platformAttachment, int timeDirection, int type); // belt, hourglass
	void addPlatform(int x, int y, int xspeed, int yspeed, int timeDirection, int id);
	void addSwitch(int x, int y, int type, int platformAttachment, int id); // and lasers

	vector<Guy*> getGuyList();
	vector<Box*> getBoxList();
	vector<Item*> getItemList();
	vector<Pickup*> getPickupList();
	vector<Platform*> getPlatformList();
	vector<Switch*> getSwitchList();

	const ObjectList* operator+(ObjectList* other);
	bool operator==(const ObjectList* other);

private:

	vector<Guy*> guyList;
	vector<Box*> boxList;
	vector<Item*> itemList;
	vector<Pickup*> pickupList;
	vector<Platform*> platformList;
	vector<Switch*> switchList;

};

