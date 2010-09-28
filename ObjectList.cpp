#include "ObjectList.h"

#include "ObjectListData.h"

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace ::std;
using namespace ::boost;
namespace hg {
// ------------ ObjectList functions ---------------
ObjectList::ObjectList() :
data_(new ObjectListData())
{
}

ObjectList::ObjectList(const ObjectList& other) :
data_(other.data_)
{
}

ObjectList::~ObjectList()
{
}

ObjectList& ObjectList::operator=(const ObjectList& other)
{
    if (this != &other) {
        this->data_ = other.data_;
    }
    return *this;
}

ObjectList::ObjectList(const MutableObjectList& other) :
data_(other.data_)
{
    data_->sortElements();
}

ObjectList& ObjectList::operator=(const MutableObjectList& other)
{
    data_ = other.data_;
    data_->sortElements();
    return *this;
}

const vector<Guy>& ObjectList::getGuyListRef() const
{
    return data_->guyList;
}

const vector<Box>& ObjectList::getBoxListRef() const
{
    return data_->boxList;
}

const vector<Button>& ObjectList::getButtonListRef() const
{
    return data_->buttonList;
}

const vector<Platform>& ObjectList::getPlatformListRef() const
{
    return data_->platformList;
}

const vector<DepartureThief<Guy> >& ObjectList::getGuyThiefListRef() const
{
    return data_->guyThiefList;
}

const vector<DepartureThief<Box> >& ObjectList::getBoxThiefListRef() const
{
    return data_->boxThiefList;
}

bool ObjectList::operator==(const hg::ObjectList& other) const
{
    return data_ == other.data_ ||
            (data_->guyList == other.data_->guyList
            && data_->boxList == other.data_->boxList
            && data_->buttonList == other.data_->buttonList
            && data_->platformList == other.data_->platformList
            && data_->guyThiefList == other.data_->guyThiefList
            && data_->boxThiefList == other.data_->boxThiefList);
}

bool ObjectList::operator!=(const hg::ObjectList& other) const
{
    return !(*this == other);
}

bool ObjectList::isEmpty() const
{
	return  data_->guyList.empty() &&
            data_->boxList.empty() &&
            data_->buttonList.empty() &&
            data_->platformList.empty() &&
            data_->guyThiefList.empty() &&
            data_->boxThiefList.empty();
}

// ---------- MutableObjectList functions ------------

MutableObjectList::MutableObjectList() :
data_(new ObjectListData())
{
}

MutableObjectList::~MutableObjectList()
{
}

MutableObjectList::MutableObjectList(const ObjectList& other) :
data_(other.data_)
{
}

MutableObjectList::MutableObjectList(const MutableObjectList& other) :
data_(other.data_)
{
}

MutableObjectList& MutableObjectList::operator=(const MutableObjectList& other)
{
    if (this != &other) {
        data_ = other.data_;
    }
    return *this;
}

void MutableObjectList::makeUnique()
{
    if (!data_.unique()) {
        data_ = boost::shared_ptr<ObjectListData>(new ObjectListData(*data_));
    }
}

void MutableObjectList::addGuy(const Guy& toCopy)
{
    makeUnique();
	data_->guyList.push_back(toCopy);
}

void MutableObjectList::addBox(const Box& toCopy)
{
    makeUnique();
	data_->boxList.push_back(toCopy);
}

void MutableObjectList::addButton(const Button& toCopy)
{
    makeUnique();
	data_->buttonList.push_back(toCopy);
}

void MutableObjectList::addPlatform(const Platform& toCopy)
{
    makeUnique();
	data_->platformList.push_back(toCopy);
}

void MutableObjectList::addGuyThief(const DepartureThief<Guy>& toCopy)
{
    makeUnique();
	data_->guyThiefList.push_back(toCopy);
}

void MutableObjectList::addBoxThief(const DepartureThief<Box>& toCopy)
{
    makeUnique();
	data_->boxThiefList.push_back(toCopy);
}

void MutableObjectList::add(const MutableObjectList& other)
{
    makeUnique();
    data_->add(*other.data_);
}

void MutableObjectList::add(const ObjectList& other)
{
    makeUnique();
    data_->add(*other.data_);
}
}//namespace hg
