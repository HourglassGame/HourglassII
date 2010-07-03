
#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST
#include "ObjectList.h"
#endif // INC_OBJECTLIST

#ifndef INC_INPUTLIST
#define INC_INPUTLIST
#include "InputList.h"
#endif // INC_INPUTLIST

#include <boost/smart_ptr.hpp>
#include <vector>
using namespace std;

class PhysicsEngine
{

public:

	vector<boost::shared_ptr<ObjectList>> executeFrame(boost::shared_ptr<ObjectList> arrivals, int time, int maxTime, 
		int playerGuyIndex, vector<boost::shared_ptr<InputList>> playerInput); // executes frame and returns departures
	
	int getNextPlayerFrame();

private:

	int nextPlayerFrame;

};