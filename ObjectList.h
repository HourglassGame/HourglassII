#ifndef HG_OBJECT_LIST_H
#define HG_OBJECT_LIST_H

#define BOOST_SP_DISABLE_THREADS
#include <boost/shared_ptr.hpp>

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
    
	void addGuy(const Guy& toCopy); 
	void addBox(const Box& toCopy); 
    
    inline const ::std::vector<Guy>& getGuyListRef() const {
        return data->guyList;
    }
    inline const ::std::vector<Box>& getBoxListRef() const {
        return data->boxList;
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
    struct Data;
    ::boost::shared_ptr<Data> data;
    struct Data {
        ::std::vector<Guy> guyList;
        ::std::vector<Box> boxList;
    };
};
}
#endif //HG_OBJECT_LIST_H
