#ifndef HG_OBJECT_AND_TIME
#define HG_OBJECT_AND_TIME
#include "Frame.h"
namespace hg {
	template<typename Object>
    struct ObjectAndTime
    {
        ObjectAndTime(
            Object const& nobject,
            Frame* nTime) :
                object(nobject),
                time(nTime) {}
        Object object;
        Frame* time;
    };
}
#endif //HG_OBJECT_AND_TIME
