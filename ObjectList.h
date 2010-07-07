
#ifndef INC_OBJECTS
#define INC_OBJECTS

#include "Guy.h"
#include "Box.h"
#include "Item.h"
#include "Pickup.h"
#include "Platform.h"
#include "Switch.h"

#endif // INC_OBJECTS

#include <boost/smart_ptr.hpp>
#include <vector>
using namespace std;

// Object list stores all data sent between frames or to rendering engine
class ObjectList  
{
public:
	ObjectList();
	
	void addGuy(int x, int y, int xspeed, int yspeed, int width, int height, int timeDirection, bool boxCarrying, int boxCarrySize, int boxCarryDirection, int relativeIndex, int subimage);
	void addBox(int x, int y, int xspeed, int yspeed, int size, int timeDirection); // I know it's the same as Item but this way is more visible
	void addItem(int x, int y, int xspeed, int yspeed, int timeDirection, int type); // jetpack, gun
	void addPickup(int x, int y, int platformAttachment, int timeDirection, int type); // belt, hourglass
	void addPlatform(int x, int y, int xspeed, int yspeed, int timeDirection, int id);
	void addSwitch(int x, int y, int type, int platformAttachment, int id); // and lasers

	void addGuy(boost::shared_ptr<Guy> toCopy); 
	void addBox(boost::shared_ptr<Box> toCopy); 

	vector<boost::shared_ptr<Guy> > getGuyList() const;
	vector<boost::shared_ptr<Box> > getBoxList() const;
	vector<boost::shared_ptr<Item> > getItemList() const;
	vector<boost::shared_ptr<Pickup> > getPickupList() const;
	vector<boost::shared_ptr<Platform> > getPlatformList() const;
	vector<boost::shared_ptr<Switch> > getSwitchList() const;

	void add(const ObjectList& other, int relativeGuyIgnoreIndex);

	// call sortElements before comparison for correct comparison
	// call sortElements before physics step for determination guarantee 
	// in practise call sortElements after changing the ObjectList
	void sortElements();
	bool equals(const ObjectList& other) const;
	bool isEmpty();
private:

	vector<boost::shared_ptr<Guy> > guyList;
	vector<boost::shared_ptr<Box> > boxList;
	vector<boost::shared_ptr<Item> > itemList;
	vector<boost::shared_ptr<Pickup> > pickupList;
	vector<boost::shared_ptr<Platform> > platformList;
	vector<boost::shared_ptr<Switch> > switchList;

};

