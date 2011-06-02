#ifndef HG_REMOTE_DEPARTURE_EDIT_DEF
#define HG_REMOTE_DEPARTURE_EDIT_DEF
#include "RemoteDepartureEdit_decl.h"
namespace hg {
template <typename EditT, typename ObjectT>
RemoteDepartureEdit<EditT, ObjectT>::RemoteDepartureEdit(
    const PauseInitiatorID& origin,
    const ObjectT& departure,
    bool propIntoNormal) :
        origin_(origin),
        departure_(departure),
        propIntoNormal_(propIntoNormal)
{
}

template <typename EditT, typename ObjectT>
RemoteDepartureEdit<EditT, ObjectT>::RemoteDepartureEdit(const RemoteDepartureEdit<EditT, ObjectT>& other) :
        origin_(other.origin_),
        departure_(other.departure_),
        propIntoNormal_(other.propIntoNormal_)
{
}

template <typename EditT, typename ObjectT>
RemoteDepartureEdit<EditT, ObjectT>& RemoteDepartureEdit<EditT, ObjectT>::operator=(const RemoteDepartureEdit<EditT, ObjectT>& other)
{
    origin_ = other.origin_;
    departure_ = other.departure_;
    propIntoNormal_ = other.propIntoNormal_;
    return *this;
}

template <typename EditT, typename ObjectT>
const ObjectT& RemoteDepartureEdit<EditT, ObjectT>::getDeparture() const {
    return departure_;
}

template <typename EditT, typename ObjectT>
const PauseInitiatorID& RemoteDepartureEdit<EditT, ObjectT>::getOrigin() const {
    return origin_;
}

template <typename EditT, typename ObjectT>
bool RemoteDepartureEdit<EditT, ObjectT>::getPropIntoNormal() const {
    return propIntoNormal_;
}

template <typename EditT, typename ObjectT>
bool RemoteDepartureEdit<EditT, ObjectT>::operator==(const RemoteDepartureEdit<EditT, ObjectT>& other) const
{
    return (origin_ == other.origin_)
           && (departure_ == other.departure_)
           && (propIntoNormal_ == other.propIntoNormal_);
}

template <typename EditT, typename ObjectT>
bool RemoteDepartureEdit<EditT, ObjectT>::operator<(const RemoteDepartureEdit<EditT, ObjectT>& other) const
{
    if (origin_ == other.origin_) {
        if (departure_ == other.departure_) {
            if (propIntoNormal_ == other.propIntoNormal_) {
                return false;
            }
            else {
                return propIntoNormal_ < other.propIntoNormal_;
            }
        }
        else {
            return departure_ < other.departure_;
        }
    }
    else {
        return origin_ < other.origin_;
    }
}
}

#endif //HG_REMOTE_DEPARTURE_EDIT_DEF
