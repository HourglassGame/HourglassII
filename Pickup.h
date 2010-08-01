#ifndef HG_PICKUP_H
#define HG_PICKUP_H
#define BOOST_SP_DISABLE_THREADS
#include <boost/smart_ptr.hpp>
#include "TimeDirection.h"
namespace hg {
class Pickup
{

public:
    Pickup(int x, int y, hg::TimeDirection timeDirection, int type ,int platformAttachment);
	/*
    int getX();
    int getY();
    int getXspeed();
    int getYspeed();
	*/
	// exectute physics
    
private:
	int x;
	int y;

    hg::TimeDirection timeDirection;
	int type;
	int platformAttachment;
     
};
}
#endif //HG_PICKUP_H
