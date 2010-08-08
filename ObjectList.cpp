#include "ObjectList.h"

#include <vector>
#include <algorithm>
using namespace std;
using namespace hg;
void ObjectList::add(const ObjectList& other)
{	
	for (std::vector<hg::Guy>::const_iterator it(other.guyList.begin()), end(other.guyList.end()); it!=end; ++it)
	{
        guyList.push_back(*it);
	}

	for (std::vector<hg::Box>::const_iterator it(other.boxList.begin()), end(other.boxList.end()); it!=end; ++it)
	{
        boxList.push_back(*it);
	}
    sortElements();
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
	if (guyList.size() != other.guyList.size() || boxList.size() != other.boxList.size() )
	{
		return false;
	}
    
	for (std::vector<hg::Guy>::const_iterator it(guyList.begin()),
         oit(other.guyList.begin()),end(guyList.end()); 
         it != end;
         ++it, ++oit)
	{
		if (*it != *oit)
		{
			return false;
		}
	}

	for (std::vector<hg::Box>::const_iterator it(boxList.begin()),
         oit(other.boxList.begin()),end(boxList.end()); 
         it != end;
         ++it, ++oit)
	{
		if (*it != *oit)
		{
			return false;
		}
	}
	return true;
}

bool ObjectList::isEmpty()
{
	return (guyList.size() == 0 && boxList.size() == 0);
}


void ObjectList::sortElements()
{
	std::sort(guyList.begin(), guyList.end());
	std::sort(boxList.begin(), boxList.end());
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
