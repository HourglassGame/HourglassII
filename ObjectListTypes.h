#ifndef HG_ARRIVAL_DEPARTURE_TYPES_H
#define HG_ARRIVAL_DEPARTURE_TYPES_H

#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "TriggerData.h"
#include "RemoteDepartureEdit.h"

#include <boost/fusion/container/vector.hpp>
namespace hg {
    typedef
    boost::fusion::vector<
        Guy,
        Box,
        Button,
        Platform,
        Portal,
        TriggerData,
        RemoteDepartureEdit<Thief,Box>,
        RemoteDepartureEdit<Extra,Box>,
        RemoteDepartureEdit<Extra,Guy> > Normal;

    typedef boost::fusion::vector<> Edit;
}
#endif //HG_ARRIVAL_DEPARTURE_TYPES_H

