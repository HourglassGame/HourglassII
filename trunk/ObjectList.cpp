#include "ObjectList.h"

#include "ObjectListData.h"

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace ::std;
using namespace ::hg;
using namespace ::boost;

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

bool ObjectList::operator==(const hg::ObjectList& other) const
{
    return data_ == other.data_ ||
            data_->guyList == other.data_->guyList
            && data_->boxList == other.data_->boxList;
}

bool ObjectList::operator!=(const hg::ObjectList& other) const
{
    return !(*this == other);
}

bool ObjectList::isEmpty() const
{
	return data_->guyList.empty() && data_->boxList.empty();
}

// ---------- MutableObjectList functions ------------

MutableObjectList::MutableObjectList() :
data_(new ObjectListData())
{
}

MutableObjectList::~MutableObjectList()
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
        data_ = shared_ptr<ObjectListData>(new ObjectListData(*data_));
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
