#ifndef HG_EXPLOSION_EFFECT_H
#define HG_EXPLOSION_EFFECT_H

#include "hg/TimeEngine/ArrivalDepartures/TimeDirection.h"

namespace hg {
	struct ExplosionEffect final {
		int x;
		int y;
		int radius;
		TimeDirection direction;
	};
}
#endif //HG_EXPLOSION_EFFECT_H