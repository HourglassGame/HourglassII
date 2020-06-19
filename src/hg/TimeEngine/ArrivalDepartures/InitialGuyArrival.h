#ifndef HG_INITIAL_GUY_ARRIVAL_H
#define HG_INITIAL_GUY_ARRIVAL_H
#include "Guy.h"
namespace hg {
struct InitialGuyArrival final {
	InitialGuyArrival(int narrivalTime, Guy const &narrival)
		: arrivalTime(narrivalTime), arrival(narrival) {}
	int arrivalTime;
	Guy arrival;
};
}
#endif //HG_INITIAL_GUY_ARRIVAL_H
