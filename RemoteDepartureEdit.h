#ifndef HG_REMOTE_DEPARTURE_EDIT_H
#define HG_REMOTE_DEPARTURE_EDIT_H
#include "PauseInitiatorID.h"
namespace hg {
struct Extra {};
struct Thief {};
template <typename EditT, typename ObjectT>
class RemoteDepartureEdit
{
public:
    RemoteDepartureEdit(const PauseInitiatorID& origin, const ObjectT& departure, bool propIntoNormal);
    RemoteDepartureEdit(const RemoteDepartureEdit<EditT, ObjectT>& other);
    RemoteDepartureEdit& operator=(const RemoteDepartureEdit<EditT, ObjectT>& other);

    const ObjectT& getDeparture() const;
    const PauseInitiatorID& getOrigin() const;
    bool getPropIntoNormal() const;
    
    bool operator!=(const RemoteDepartureEdit<EditT, ObjectT>& other) const;
    bool operator==(const RemoteDepartureEdit<EditT, ObjectT>& other) const;

    bool operator<(const RemoteDepartureEdit<EditT, ObjectT>& second) const;
private:
    PauseInitiatorID origin_;
    ObjectT departure_;
    bool propIntoNormal_;
};
}
#endif //HG_REMOTE_DEPARTURE_EDIT_H
