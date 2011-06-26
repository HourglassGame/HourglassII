#ifndef HG_REMOTE_DEPARTURE_EDIT_H
#define HG_REMOTE_DEPARTURE_EDIT_H
#include "PauseInitiatorID.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
namespace hg {
struct Extra {};
struct Thief {};
template <typename EditT, typename ObjectT>
class RemoteDepartureEdit : boost::totally_ordered<RemoteDepartureEdit<EditT, ObjectT> >
{
public:
    RemoteDepartureEdit(const PauseInitiatorID& origin, const ObjectT& departure, bool propIntoNormal);

    const ObjectT& getDeparture() const;
    const PauseInitiatorID& getOrigin() const;
    bool getPropIntoNormal() const;
    
    bool operator==(const RemoteDepartureEdit<EditT, ObjectT>& other) const;

    bool operator<(const RemoteDepartureEdit<EditT, ObjectT>& other) const;
private:
    PauseInitiatorID origin_;
    ObjectT departure_;
    bool propIntoNormal_;
};

template <typename EditT, typename ObjectT>
class RemoteDepartureEditConstPtr : boost::totally_ordered<RemoteDepartureEditConstPtr<EditT, ObjectT> >
{
public:
    RemoteDepartureEditConstPtr(RemoteDepartureEdit<EditT, ObjectT> const& edit) : edit_(&edit) {}
    typedef RemoteDepartureEdit<EditT, ObjectT> base_type;
    RemoteDepartureEdit<EditT, ObjectT> const& get() const  { return *edit_; }
    ObjectT const& getDeparture() const {return edit_->getDeparture(); }
    PauseInitiatorID const& getOrigin() const {return edit_->getOrigin(); }
    bool getPropIntoNormal() const {return edit_->getPropIntoNormal(); }
    
    bool operator==(const RemoteDepartureEditConstPtr<EditT, ObjectT>& other) const {
        return *edit_ == *other.edit_;
    }

    bool operator<(const RemoteDepartureEditConstPtr<EditT, ObjectT>& other) const {
        return *edit_ < *other.edit_;
    }
private:
    RemoteDepartureEdit<EditT, ObjectT> const* edit_;
};
template<typename EditT, typename ObjectT>
struct ConstPtr_of<RemoteDepartureEdit<EditT, ObjectT> > {
    typedef RemoteDepartureEditConstPtr<EditT, ObjectT> type;
};
}
#endif //HG_REMOTE_DEPARTURE_EDIT_H
