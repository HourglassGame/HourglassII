#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H
#include "PhysicsEngine.h"
#include "TimeObjectListList.h"

#include "InputList.h"

#include "UpdateStackMember.h"
#include "ArrivalDepartureMap.h"

#include <boost/ptr_container/ptr_map.hpp>
#define BOOST_SP_DISABLE_THREADS
#include <boost/smart_ptr.hpp>
#include <vector>
#include <stack>
namespace hg {
class TimeEngine
{

public:
	TimeEngine(int newTimeLength, std::vector<std::vector<bool> > wallmap, int newWallSize, int newGravity);
    // returns if the level creator not a foo
    // Call before trying to use level
	bool checkConstistencyAndPropagateLevel(boost::shared_ptr<hg::ObjectList> initialObjects, int guyStartTime);

    std::vector<boost::shared_ptr<hg::ObjectList> > getNextPlayerFrame(boost::shared_ptr<hg::InputList> newInputData);

	boost::shared_ptr<hg::ObjectList> getLastArrival(int time); // just for debug I think
	
private:
	
	// updates the frame using new arrivals. departures are checked and added to frameUpdateStack if different
	bool updateFrame(int frameId);
    // runs the frame update stack until empty
	bool executeFrameUpdateStack();
    
    // 2D array of departures(y) and arrival(x)
	boost::shared_ptr<hg::ArrivalDepartureMap> arrivalDeparturePair;
	int arrivalFrames; // size of arrival array
	int departuresFrames; // size of departure array

	int permanentDepatureIndex; // index in arrivalDeparturePair which stores permanent departures 

	int timeLineLength; // size of playable timeline

	//int playerGuyIndex; // the relative guy index that most recently recived input
	int nextPlayerFrame; // frame that has the next guy index in it
	int currentPlayerFrame; // frame that has the current guy index in it
	bool updateStartFirst; // either start from the end or beginning of time for propgation
	
	boost::shared_ptr<hg::ObjectList> currentPlayerFrameData; // draw data to be returned by getNextPlayerFrame
	
    std::vector<boost::shared_ptr<hg::InputList> > playerInput; // stores all player input



    // executes physics of a frame and a few extra things
	boost::shared_ptr<hg::PhysicsEngine> physics;
    
    // previous arrivals for frames, used for paradox checking so may contain nothing
    std::vector<std::vector<boost::shared_ptr<hg::ObjectList> > > previousArrival;
    
    // the last arrival for frames, always contains 1 object list for each frame
    std::vector<boost::shared_ptr<hg::ObjectList> > lastArrival;

    // stores frames that require updating in FILO
    std::vector<hg::UpdateStackMember> frameUpdateStack; 

};
}
#endif //HG_TIME_ENGINE_H
