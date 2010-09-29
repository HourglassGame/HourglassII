#ifndef HG_DEPARTURE_THIEF_H
#define HG_DEPARTURE_THIEF_H
#include "PauseInitiatorID.h"

namespace hg {
    template <class Type> class RemoteDepartureEdit
    {
    public:
        RemoteDepartureEdit(PauseInitiatorID origin, Type departure, bool propIntoNormal);
        RemoteDepartureEdit<Type>(const RemoteDepartureEdit<Type>& other);
        ~RemoteDepartureEdit();
        RemoteDepartureEdit& operator=(const RemoteDepartureEdit<Type>& other);

        const Type& getDeparture() const {return data->departure;}
        const PauseInitiatorID& getOrigin() const {return data->origin;}
        const bool& getPropIntoNormal() const {return data->propIntoNormal;}

        bool operator!=(const RemoteDepartureEdit<Type>& other) const;
        bool operator==(const RemoteDepartureEdit<Type>& other) const;

        bool operator<(const RemoteDepartureEdit<Type>& second) const;

    private:

        struct Data;
        void decrementCount();
        mutable int* referenceCount;

        Data* data;

        struct Data {
            Data(PauseInitiatorID norigin,
                 Type ndeparture,
                 bool npropIntoNormal
            ):
            origin(norigin),
            departure(ndeparture),
            propIntoNormal(npropIntoNormal)
            {
            }

            PauseInitiatorID origin;
            Type departure;
            bool propIntoNormal;
        };

    };
}
#endif //HG_DEPARTURE_THIEF_H
