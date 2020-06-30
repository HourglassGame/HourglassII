#ifndef HG_ARRIVAL_DEPARTURE_TYPES_H
#define HG_ARRIVAL_DEPARTURE_TYPES_H

#include "Guy.h"
#include "Box.h"
#include "Explosion.h"
#include "TriggerData.h"
#include "GlitzPersister.h"

#include <boost/fusion/container/vector.hpp>
namespace hg {
	typedef
		boost::fusion::vector<
			Guy,
			Box,
			Explosion,
			TriggerData,
			GlitzPersister
		> Normal;
	typedef
		boost::fusion::vector<
			Box
		> NonGuyDynamic;
}
#endif //HG_ARRIVAL_DEPARTURE_TYPES_H

