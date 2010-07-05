
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

	PhysicsEngine(int newTimeLineLength, vector<boost::shared_ptr<InputList>> playerInputRef, vector<vector<bool>> newWallmap);

	vector<boost::shared_ptr<ObjectList>> executeFrame(boost::shared_ptr<ObjectList> arrivals, int time, int playerGuyIndex); // executes frame and returns departures
	

	int getNextPlayerFrame();
	int getPlayerDirection();

private:

	void crappyBoxCollisionAlogorithm();

	int timeLineLength;
	vector<boost::shared_ptr<InputList>> playerInput;

	vector<vector<bool>> wallmap;

	// these are updated from guy with relative index == playerGuyIndex when the frame is executed
	int nextPlayerFrame; // frame that the player departed for
	int playerDirection; // time direction of player in frame

};