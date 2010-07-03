
#ifndef INC_PHYSICSENGINE
#define INC_PHYSICSENGINE
#include "PhysicsEngine.h"
#endif // INC_PHYSICSENGINE

#ifndef INC_INPUTLIST
#define INC_INPUTLIST
#include "InputList.h"
#endif // INC_INPUTLIST

#include "UpdateStackMember.h"

#include <boost/smart_ptr.hpp>
#include <vector>
#include <stack>
using namespace std;

class TimeEngine
{

public:

	TimeEngine(int newFrameCount);

	boost::shared_ptr<ObjectList> getNextPlayerFrame(boost::shared_ptr<InputList> newInputData);
	
private:

	int frameCount; // how many time frames in the timeline

	// updates the frame using new arrivals. departures are checked and added to frameUpdateStack if different
	void updateFrame(int frameId); 
	
	void executeFrameUpdateStack(); // runs the frame update stack until empty

	int playerGuyIndex; // the relative guy index that most recently recived input
	int nextPlayerFrame; // frame that has the next guy index in it
	int currentPlayerFrame; // frame that has the current guy index in it
	
	boost::shared_ptr<ObjectList> currentPlayerFrameData; // draw data to be returned by getNextPlayerFrame
	
	vector<boost::shared_ptr<InputList>> playerInput; // stores all player input

	boost::shared_ptr<PhysicsEngine> physics; // executes physics of a frame and a few extra things

	vector<vector<boost::shared_ptr<ObjectList>>> arrivalDeparturePair; // 2D array of departures(y) and arrival(x)

	vector<vector<boost::shared_ptr<ObjectList>>> previousArrival; // 1D array storing the last departure sent for each frame

	vector<boost::shared_ptr<UpdateStackMember>> frameUpdateStack; // stores frames that require updating in FILO 

};

