#include "ObjectList.h"

#include <vector>
#include <algorithm>
#include <cassert>

using namespace ::std;
using namespace ::hg;

ObjectList::ObjectList() :
guyList(),
boxList()
{
}

ObjectList::ObjectList(const ObjectList& other) :
guyList(other.guyList),
boxList(other.boxList)
{
}

ObjectList::~ObjectList()
{
}

ObjectList& ObjectList::operator=(const ObjectList& other)
{
    if (this != &other) {
        guyList = other.guyList;
        boxList = other.boxList;
    }
    return *this;
}

void ObjectList::add(const ObjectList& other)
{
    guyList.insert(guyList.end(),other.guyList.begin(),other.guyList.end());
    boxList.insert(boxList.end(),other.boxList.begin(),other.boxList.end());
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
	if (guyList.size() != other.guyList.size() || boxList.size() != other.boxList.size())
	{
		return false;
	}
    assert(guyList.size() == other.guyList.size());
    assert(boxList.size() == other.boxList.size());
    return equal(guyList.begin(), guyList.end(), other.guyList.begin()) 
            && equal(boxList.begin(), boxList.end(), other.boxList.begin());
}

bool ObjectList::isEmpty() const
{
	return guyList.empty() && boxList.empty();
}


void ObjectList::sortElements()
{
	sort(guyList.begin(), guyList.end());
	sort(boxList.begin(), boxList.end());
}

// Single Element addition
void ObjectList::addGuy(const Guy& toCopy)
{
	guyList.push_back(toCopy);
}

void ObjectList::addBox(const Box& toCopy)
{
	boxList.push_back(toCopy);
}
