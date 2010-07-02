
#ifndef INC_PHYSICSENGINE
#define INC_PHYSICSENGINE

#include "ObjectList.h"

#endif // INC_PHYSICSENGINE

#include <vector>
using namespace std;

class PhysicsEngine
{

public:

	static ObjectList* executeFrame(ObjectList* arrivals); // executes frame and returns departures
	
private:


};