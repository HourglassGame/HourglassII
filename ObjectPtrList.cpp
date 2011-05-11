#include "ObjectList.h"
#include "ObjectListHelpers.h"

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/vector_tie.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <functional>

namespace hg {
ObjectPtrList::ObjectPtrList() :
        objectPtrList_()
{
}
ObjectPtrList::ObjectPtrList(const ObjectPtrList& other):
        objectPtrList_(other.objectPtrList_)
{
}
ObjectPtrList& ObjectPtrList::operator=(const ObjectPtrList& other)
{
    objectPtrList_ = other.objectPtrList_;
    return *this;
}

boost::indirected_range<const std::vector<const Guy*> > ObjectPtrList::getGuyListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::guyList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const Box*> > ObjectPtrList::getBoxListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::boxList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const Button*> > ObjectPtrList::getButtonListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::buttonList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const Platform*> > ObjectPtrList::getPlatformListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::platformList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const Portal*> > ObjectPtrList::getPortalListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::portalList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const TriggerData*> > ObjectPtrList::getTriggerDataListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::triggerDataList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const RemoteDepartureEdit<Box>*> > ObjectPtrList::getBoxThiefListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::boxThiefList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const RemoteDepartureEdit<Box>*> > ObjectPtrList::getBoxExtraListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::boxExtraList>(objectPtrList_));
}
boost::indirected_range<const std::vector<const RemoteDepartureEdit<Guy>*> > ObjectPtrList::getGuyExtraListRef() const
{
    return boost::adaptors::indirect(boost::fusion::at_c<object_list_detail::guyExtraList>(objectPtrList_));
}

namespace {
template<typename T>
struct DereferenceLess {
	bool operator()(const T& l, const T& r) const
	{
		return *l < *r;
	}
};
template <typename T>
struct AddressOf : std::unary_function<T&, T*>
{
    T* operator()(T& t) const
    {
        return &t;
    }
};
struct InsertAddresses
{
    template<typename Container, typename SinglePassRange>
    void operator()(Container& toInsertInto, const SinglePassRange& toInsert) const
    {
        boost::push_back(
        	toInsertInto,
        	toInsert |
        		boost::adaptors::transformed(
        			AddressOf<const typename boost::range_value<SinglePassRange>::type>()));
    }
};
}//namespace

void ObjectPtrList::add(const ObjectList& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectPtrList_, other.objectList_), InsertAddresses());
}
//MUST CALL this before calling operator== on this ObjectPtrList
void ObjectPtrList::sort()
{
    boost::fusion::for_each(objectPtrList_, Sort<DereferenceLess>());
}
void ObjectPtrList::swap(ObjectPtrList& other)
{
    using namespace boost::fusion;
    n_ary_for_each(vector_tie(objectPtrList_, other.objectPtrList_), Swap());
}
}//namespace hg
