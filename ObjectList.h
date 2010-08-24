#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H

#include "Guy.h"
#include "Box.h"

#include <vector>
#include "TimeDirection.h"
namespace hg {
// Object list stores all data sent between frames or to rendering engine
class ObjectList  
{
public:
    ObjectList();
    
    ~ObjectList();
    ObjectList(const ObjectList& other);
    ObjectList& operator=(const ObjectList& other);
    
	void addGuy(const Guy& toCopy); 
	void addBox(const Box& toCopy); 
    
    inline const ::std::vector<Guy>& getGuyListRef() const {
        return guyList;
    }
    inline const ::std::vector<Box>& getBoxListRef() const {
        return boxList;
    }
    //Add other ref getters as needed
    
	void add(const ObjectList& other);

	// call sortElements before comparison for correct comparison
	// call sortElements before physics step for determination guarantee 
	// in practise call sortElements after changing the ObjectList
	void sortElements();
	bool equals(const ObjectList& other) const;
    bool operator==(const ObjectList& other) const;
    bool operator!=(const ObjectList& other) const;
	bool isEmpty() const;
private:
    ::std::vector<Guy> guyList;
    ::std::vector<Box> boxList;
};
}
#endif //HG_OBJECT_LIST_H
