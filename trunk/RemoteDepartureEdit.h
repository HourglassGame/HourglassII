#ifndef HG_DEPARTURE_THIEF_H
#define HG_DEPARTURE_THIEF_H
#include "PauseInitiatorID.h"

namespace hg {
template <class Type> class RemoteDepartureEdit
{
public:
    RemoteDepartureEdit(PauseInitiatorID origin, Type departure, bool propIntoNormal);
    RemoteDepartureEdit(const RemoteDepartureEdit<Type>& other);
    RemoteDepartureEdit& operator=(const RemoteDepartureEdit<Type>& other);

    const Type& getDeparture() const {
        return departure_;
    }
    const PauseInitiatorID& getOrigin() const {
        return origin_;
    }
    const bool getPropIntoNormal() const {
        return propIntoNormal_;
    }

    bool operator!=(const RemoteDepartureEdit<Type>& other) const;
    bool operator==(const RemoteDepartureEdit<Type>& other) const;

    bool operator<(const RemoteDepartureEdit<Type>& second) const;

private:

    PauseInitiatorID origin_;
    Type departure_;
    bool propIntoNormal_;
};
}

#include "RemoteDepartureEditImpl.h"
#endif //HG_DEPARTURE_THIEF_H
