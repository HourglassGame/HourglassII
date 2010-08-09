#include "ObjectList.h"

#include <vector>
#include <algorithm>
#include <cassert>

//Can't leave the asserts in because I am doing this stuff legitimately in places
//But ... YOU HAVE BEEN WARNED!

using namespace std;
using namespace hg;

ObjectList::ObjectList() :
data(new Data)
{
}

void ObjectList::add(const ObjectList& other)
{	
    //assert(data.unique() && "Are you sure you want to be doing this? "
    //            "You could be modifying something in an unrelated part of the programme");
    data->guyList.insert(data->guyList.end(),other.data->guyList.begin(),other.data->guyList.end());
    data->boxList.insert(data->boxList.end(),other.data->boxList.begin(),other.data->boxList.end());
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
	if (data->guyList.size() != other.data->guyList.size() || data->boxList.size() != other.data->boxList.size())
	{
		return false;
	}
    return std::equal(data->guyList.begin(), data->guyList.end(), other.data->guyList.begin()) 
            && std::equal(data->boxList.begin(), data->boxList.end(), other.data->boxList.begin());
}

bool ObjectList::isEmpty() const
{
	return data->guyList.empty() && data->boxList.empty();
}


void ObjectList::sortElements()
{
    //assert(data.unique() && "Are you sure you want to be doing this? "
    //    "You could be modifying something in an unrelated part of the programme");
	std::sort(data->guyList.begin(), data->guyList.end());
	std::sort(data->boxList.begin(), data->boxList.end());
}

// Single Element addition
void ObjectList::addGuy(const Guy& toCopy)
{
    //assert(data.unique() && "Are you sure you want to be doing this? "
    //       "You could be modifying something in an unrelated part of the programme");
	data->guyList.push_back(toCopy);
}

void ObjectList::addBox(const Box& toCopy)
{
    //assert(data.unique() && "Are you sure you want to be doing this? "
    //       "You could be modifying something in an unrelated part of the programme");
	data->boxList.push_back(toCopy);
}

