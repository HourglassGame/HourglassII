#ifndef HG_DEPARTURE_THIEF_H
#define HG_DEPARTURE_THIEF_H
#include "PauseInitiatorID.h"

namespace hg {
    template <class Type> class DepartureThief
    {
    public:
        DepartureThief(PauseInitiatorID origin, Type departure);
        DepartureThief<Type>(const DepartureThief<Type>& other);
        ~DepartureThief();
        DepartureThief& operator=(const DepartureThief<Type>& other);

        const Type& getDeparture() const {return data->departure;}
        const PauseInitiatorID& getOrigin() const {return data->origin;}

        bool operator!=(const DepartureThief<Type>& other) const;
        bool operator==(const DepartureThief<Type>& other) const;

        bool operator<(const DepartureThief<Type>& second) const;

    private:

        struct Data;
        void decrementCount();
        mutable int* referenceCount;

        Data* data;

        struct Data {
            Data(PauseInitiatorID norigin,
                 Type ndeparture
            ):
            origin(norigin),
            departure(ndeparture)
            {
            }

            PauseInitiatorID origin;
            Type departure;
        };

    };
}
#endif //HG_DEPARTURE_THIEF_H
