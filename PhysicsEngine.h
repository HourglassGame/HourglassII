
#ifndef INC_OBJECTLIST
#define INC_OBJECTLIST
#include "ObjectList.h"
#endif // INC_OBJECTLIST

#ifndef INC_INPUTLIST
#define INC_INPUTLIST
#include "InputList.h"
#endif // INC_INPUTLIST

#ifndef INC_TIMEOBJECTLISTLIST
#define INC_TIMEOBJECTLISTLIST
#include "TimeObjectListList.h"
#endif // INC_TIMEOBJECTLISTLIST

#include <boost/smart_ptr.hpp>
#include <vector>
using namespace std;

class PhysicsEngine
{

public:

	PhysicsEngine(int newTimeLineLength, vector<vector<bool> > newWallmap, int newWallSize, int newGravity);

	boost::shared_ptr<TimeObjectListList> executeFrame(const ObjectList& arrivals, int time, int playerGuyIndex, vector<boost::shared_ptr<InputList> > playerInput); // executes frame and returns departures

	int getNextPlayerFrame();
	int getPlayerDirection();

private:

	void crappyBoxCollisionAlogorithm(vector<boost::shared_ptr<Box> > oldBoxList);
	void guyStep(vector<boost::shared_ptr<Guy> > oldGuyListt, int playerGuyIndex, int time, vector<boost::shared_ptr<InputList> > playerInput, boost::shared_ptr<TimeObjectListList> departures);

	bool wallAt(int x, int y);

	int timeLineLength;

	vector<vector<bool> > wallmap;

	// these are updated from guy with relative index == playerGuyIndex when the frame is executed
	int nextPlayerFrame; // frame that the player departed for
	int playerDirection; // time direction of player in frame
	int gravity;
	int wallSize;

	vector<boost::shared_ptr<Box> > nextBox;
	vector<int> nextBoxTime;
	vector<bool> supportedBox;
};