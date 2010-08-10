#include "ObjectList.h"

#include <vector>
#include <algorithm>
#include <cassert>

//Watch out, this shit is shared and you could end up shooting yourself in the foot really hard
//It is ok at the moment, and even good in places but... YOU HAVE BEEN WARNED!

using namespace ::std;
using namespace ::hg;

ObjectList::ObjectList() :
referenceCount(new int(1)),
data(new Data)
{
}

ObjectList::ObjectList(const ObjectList& other) :
referenceCount(&++(*other.referenceCount)),
data(other.data)
{
}

ObjectList::~ObjectList()
{
    decrementCount();
}

void ObjectList::decrementCount()
{
    if(--(*referenceCount) == 0) {
        delete referenceCount;
        delete data;
    }
}

ObjectList& ObjectList::operator=(const ObjectList& other)
{
    if (other.data != data) {
        decrementCount();
        referenceCount = other.referenceCount;
        data = other.data;
        ++(*referenceCount);
    }
    return *this;
}

void ObjectList::add(const ObjectList& other)
{
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
    assert(data->guyList.size() == other.data->guyList.size());
    assert(data->boxList.size() == other.data->boxList.size());
    return equal(data->guyList.begin(), data->guyList.end(), other.data->guyList.begin()) 
    && equal(data->boxList.begin(), data->boxList.end(), other.data->boxList.begin());
}

bool ObjectList::isEmpty() const
{
	return data->guyList.empty() && data->boxList.empty();
}


void ObjectList::sortElements()
{
	sort(data->guyList.begin(), data->guyList.end());
	sort(data->boxList.begin(), data->boxList.end());
}

// Single Element addition
void ObjectList::addGuy(const Guy& toCopy)
{
	data->guyList.push_back(toCopy);
}

void ObjectList::addBox(const Box& toCopy)
{
	data->boxList.push_back(toCopy);
}
