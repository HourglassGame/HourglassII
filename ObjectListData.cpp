#include "ObjectListData.h"
#include <algorithm>
using namespace ::hg;
using namespace ::std;
ObjectListData::ObjectListData() :
guyList(),
boxList()
{
}

ObjectListData::ObjectListData(const ObjectListData& other) :
guyList(other.guyList),
boxList(other.boxList)
{
}

static bool containsNoGuysWithEqualRelativeIndices(const vector<Guy>& guyList) {
    vector<size_t> relativeIndices;
    relativeIndices.reserve(guyList.size());
    
    for (vector<Guy>::const_iterator it(guyList.begin()), end(guyList.end()); it != end; ++it) {
        relativeIndices.push_back(it->getRelativeIndex());
    }
    
    return unique(relativeIndices.begin(), relativeIndices.end()) == relativeIndices.end();
}

void ObjectListData::sortElements()
{
	sort(guyList.begin(), guyList.end());
    assert(containsNoGuysWithEqualRelativeIndices(guyList) && "If the list contains guys with equal relative index then "
                                                                     "the sort order is non-deterministic, potentially leading "
                                                                     "equal objectLists being found to be different");
	sort(boxList.begin(), boxList.end());
}

void ObjectListData::add(const ObjectListData& other)
{
    guyList.insert(guyList.end(),other.guyList.begin(),other.guyList.end());
    boxList.insert(boxList.end(),other.boxList.begin(),other.boxList.end());
}
