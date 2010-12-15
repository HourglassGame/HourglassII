#include "ObjectListData.h"

#include <boost/foreach.hpp>

#include <iostream>
#include <algorithm>

#include <cassert>

#define foreach BOOST_FOREACH

using namespace ::std;
namespace hg {
ObjectListData::ObjectListData() :
guyList(),
boxList(),
buttonList(),
platformList(),
portalList(),
guyThiefList(),
boxThiefList(),
guyExtraList(),
boxExtraList()
{
}

ObjectListData::ObjectListData(const ObjectListData& other) :
guyList(other.guyList),
boxList(other.boxList),
buttonList(other.buttonList),
platformList(other.platformList),
portalList(other.portalList),
boxThiefList(other.boxThiefList),
guyExtraList(other.guyExtraList),
boxExtraList(other.boxExtraList)
{
}

static bool containsNoGuysWithEqualRelativeIndices(const vector<Guy>& guyList) {
    vector<size_t> relativeIndices;
    relativeIndices.reserve(guyList.size());

    for (vector<Guy>::const_iterator it(guyList.begin()), end(guyList.end()); it != end; ++it) {
        if (it->getRelativeIndex() != -1)
        {
            relativeIndices.push_back(it->getRelativeIndex());
        }
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
	sort(buttonList.begin(), buttonList.end());
	sort(platformList.begin(), platformList.end());
	sort(portalList.begin(), portalList.end());
	sort(boxThiefList.begin(), boxThiefList.end());
	sort(guyExtraList.begin(), guyExtraList.end());
	sort(boxExtraList.begin(), boxExtraList.end());
}

void ObjectListData::add(const ObjectListData& other)
{
    guyList.insert(guyList.end(),other.guyList.begin(),other.guyList.end());
    boxList.insert(boxList.end(),other.boxList.begin(),other.boxList.end());
    buttonList.insert(buttonList.end(),other.buttonList.begin(),other.buttonList.end());
    platformList.insert(platformList.end(),other.platformList.begin(),other.platformList.end());
    portalList.insert(portalList.end(),other.portalList.begin(),other.portalList.end());
    boxThiefList.insert(boxThiefList.end(),other.boxThiefList.begin(),other.boxThiefList.end());
    guyExtraList.insert(guyExtraList.end(),other.guyExtraList.begin(),other.guyExtraList.end());
    boxExtraList.insert(boxExtraList.end(),other.boxExtraList.begin(),other.boxExtraList.end());
}
}//namespace hg
