#ifndef HG_REMOTE_DEPARTURE_EDIT_H
#define HG_REMOTE_DEPARTURE_EDIT_H
#include "PauseInitiatorID.h"

namespace hg {
template <class Type> class RemoteDepartureEdit
{
public:
    RemoteDepartureEdit(const PauseInitiatorID& origin, const Type& departure, bool propIntoNormal);
    RemoteDepartureEdit(const RemoteDepartureEdit<Type>& other);
    RemoteDepartureEdit& operator=(const RemoteDepartureEdit<Type>& other);

    const Type& getDeparture() const;
    const PauseInitiatorID& getOrigin() const;
    const bool getPropIntoNormal() const;
    
    bool operator!=(const RemoteDepartureEdit<Type>& other) const;
    bool operator==(const RemoteDepartureEdit<Type>& other) const;

    bool operator<(const RemoteDepartureEdit<Type>& second) const;
private:
    PauseInitiatorID origin_;
    Type departure_;
    bool propIntoNormal_;
};
}
#endif //HG_REMOTE_DEPARTURE_EDIT_H
