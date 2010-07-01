
#ifndef INC_PHYSICSENGINE
#define INC_PHYSICSENGINE

#include "ObjectList.h"

#include "Guy.h"
#include "Box.h"
#include "Item.h"
#include "Pickup.h"
#include "Platform.h"
#include "Switch.h"

#endif // INC_PHYSICSENGINE

#include <vector>
using namespace std;

class PhysicsEngine
{

public:
	PhysicsEngine();

	static ObjectList executeFrame(ObjectList arrivals); // executes frame and returns departures
	
private:


};