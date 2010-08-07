#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H
#include "Guy.h"
#include "Box.h"
#include "Item.h"
#include "Pickup.h"
#include "Platform.h"
#include "Switch.h"
#include <vector>
#include "TimeDirection.h"
namespace hg {
// Object list stores all data sent between frames or to rendering engine
class ObjectList  
{
public:
	void addGuy(int x, int y, int xspeed, int yspeed, int width, int height,
                hg::TimeDirection timeDirection, bool boxCarrying, int boxCarrySize,
                hg::TimeDirection boxCarryDirection, int relativeIndex, int subimage);
	void addBox(int x, int y, int xspeed, int yspeed, int size, hg::TimeDirection timeDirection); // I know it's the same as Item but this way is more visible
	void addItem(int x, int y, int xspeed, int yspeed, hg::TimeDirection timeDirection, int type); // jetpack, gun
	void addPickup(int x, int y, int platformAttachment, hg::TimeDirection timeDirection, int type); // belt, hourglass
	void addPlatform(int x, int y, int xspeed, int yspeed, hg::TimeDirection timeDirection, int id);
	void addSwitch(int x, int y, int type, int platformAttachment, int id); // and lasers

	void addGuy(const hg::Guy& toCopy); 
	void addBox(const hg::Box& toCopy); 

    inline const std::vector<hg::Guy>& getGuyListRef() const {
        return guyList;
    }
    inline const std::vector<hg::Box>& getBoxListRef() const {
        return boxList;
    }
    //Add other ref getters as needed
    
	void add(const hg::ObjectList& other, int relativeGuyIgnoreIndex);

	// call sortElements before comparison for correct comparison
	// call sortElements before physics step for determination guarantee 
	// in practise call sortElements after changing the ObjectList
	void sortElements();
	bool equals(const hg::ObjectList& other) const;
    bool operator==(const hg::ObjectList& other) const;
    bool operator!=(const hg::ObjectList& other) const;
	bool isEmpty();
private:

	std::vector<hg::Guy> guyList;
	std::vector<hg::Box> boxList;

};
}
#endif //HG_OBJECT_LIST_H
