#include "TimeEngine.h"

#include <vector>
#include <iostream>
using namespace std;
using namespace hg;
TimeEngine::TimeEngine(int newTimeLength, vector<vector<bool> > wallmap, int newWallSize, int newGravity) :
timeLineLength(newTimeLength),
currentPlayerFrame(-1),
arrivalFrames(newTimeLength),
departuresFrames(newTimeLength +1),
permanentDepatureIndex(newTimeLength),
arrivalDeparturePair(boost::shared_ptr<ArrivalDepartureMap>(new ArrivalDepartureMap(newTimeLength))),
physics(boost::shared_ptr<PhysicsEngine>(new PhysicsEngine(newTimeLength, wallmap, newWallSize, newGravity))),
lastArrival(0),
previousArrival(0)
{
	lastArrival.reserve(arrivalFrames);
	previousArrival.reserve(arrivalFrames);

	for (int i = 0; i < arrivalFrames; ++i)
	{
		lastArrival.push_back(boost::shared_ptr<ObjectList> (new ObjectList()));
		previousArrival.push_back(vector<boost::shared_ptr<ObjectList> >());
	}
}

bool TimeEngine::checkConstistencyAndPropagateLevel(boost::shared_ptr<ObjectList> initialObjects, int guyStartTime) // returns if the level creator not a foo
{
	/* Add initialObjects to the permanent departure to arrive at the 
	beginning or end of time depending on direction */
	bool atEnd = false;
	bool atBeginning = false;

	// boxes
	for (unsigned int i = 0; i < initialObjects->getBoxList().size(); ++i)
	{
		if (initialObjects->getBoxList()[i]->getTimeDirection() == 1)
		{
			atBeginning = true;
			arrivalDeparturePair->permanentDepartureObjectList(0)->addBox(initialObjects->getBoxList()[i]);
		}
		else
		{
			atEnd = true;
			arrivalDeparturePair->permanentDepartureObjectList(timeLineLength-1)->addBox(initialObjects->getBoxList()[i]);
		}
		
	}

	// guys
	nextPlayerFrame = guyStartTime;

	if (initialObjects->getGuyList().size() == 1)
	{
		arrivalDeparturePair->permanentDepartureObjectList(guyStartTime)->addGuy(initialObjects->getGuyList()[0]);
	}
	else
	{
		return false; // must have 1 guy in level
	}

	//** determine level

	if (atBeginning)
	{
		if (atEnd) // objects created at beginning and end. Propgate from both ends to check consistency
		{	
			// propgate from start of time first
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME, 0));
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME, timeLineLength-1));
			if (executeFrameUpdateStack())
			{
				return false;
			}

			// save gamestate
			boost::shared_ptr<ArrivalDepartureMap> tempArrivalDeparturePair;
			tempArrivalDeparturePair = arrivalDeparturePair;

			arrivalDeparturePair = boost::shared_ptr<ArrivalDepartureMap>(new ArrivalDepartureMap(timeLineLength));

			// propgate from end of time first
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME, timeLineLength-1));
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME, 0));
			if (executeFrameUpdateStack())
			{
				return false;
			}

			// compare start first with end first
			if (!(arrivalDeparturePair->equals(tempArrivalDeparturePair) ))
			{
				return false;
			}
		}
		else // only the beginning require propagation
		{
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME, 0));
			if (executeFrameUpdateStack())
			{
				return false;
			}
		}
	}
	else
	{
		if (atEnd) // only the end require propagation
		{
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME, timeLineLength-1));
			if (executeFrameUpdateStack())
			{
				return false;
			}
		}
		else // there are no created objects that require propagation
		{
			return true;
		}
	}
	return true;
}

std::vector<boost::shared_ptr<ObjectList> > TimeEngine::getNextPlayerFrame(boost::shared_ptr<InputList> newInputData)
{

	playerInput.push_back(newInputData);
	//++playerGuyIndex;

	currentPlayerFrame = nextPlayerFrame;

	frameUpdateStack.push_back
    (
        UpdateStackMember
        (
            UpdateStackMember::EXECUTE_FRAME, currentPlayerFrame
        )
    );
    
    executeFrameUpdateStack();
    return std::vector<boost::shared_ptr<ObjectList> >(1,arrivalDeparturePair->getArrivals(currentPlayerFrame, playerInput.size()));
    /*
    
	if (executeFrameUpdateStack()) // paradox occured
	{
		return frameArrivalAtUpdate;
	}
	else // paradox did not occur
	{
		return vector<boost::shared_ptr<ObjectList> >(1,currentPlayerFrameData);
	}
     */
}

bool TimeEngine::updateFrame(int frame)
{
	// get the arrivals for the physics frame by going across a row 
	boost::shared_ptr<ObjectList> arrivals = arrivalDeparturePair->getArrivals(frame, playerInput.size());

	arrivals->sortElements(); // to ensure determinism of input
	lastArrival[frame] = arrivals;

	//frameArrivalAtUpdate.push_back(arrivals);

	for (unsigned int i = 0; i < previousArrival[frame].size(); ++i)
	{
		if (previousArrival[frame][i]->equals(*arrivals.get()))
		{
			return true;
		}
	}

	// get departures for the frame
	boost::shared_ptr<TimeObjectListList> departures = physics->executeFrame(*arrivals.get(), frame, playerInput.size()-1, playerInput);

	if (frame == currentPlayerFrame)
	{
		updateStartFirst = (physics->getPlayerDirection() == 1);
		nextPlayerFrame = physics->getNextPlayerFrame();
		currentPlayerFrameData = arrivals;
	}
	

	// update departures from this frame
	departures->sortObjectLists();
	vector<int> framesThatNeedUpdating = arrivalDeparturePair->updateDeparturesFromTime(frame, departures);

	// add frames that need updating to execution stack
	if (!framesThatNeedUpdating.empty())
	{
		previousArrival[frame].push_back(lastArrival[frame]);
		frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::CLEAR_PARADOX,frame));
		// the bottom added frame must remember that it will clear paradox check data from parent frame if it finishes execution
	}

	if (updateStartFirst)
	{
		for (signed int i = framesThatNeedUpdating.size()-1; i >= 0 ; --i)
		{
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME,framesThatNeedUpdating[i]));
		}
	}
	else
	{
		for (unsigned int i = 0; i < framesThatNeedUpdating.size(); ++i)
		{
			frameUpdateStack.push_back(UpdateStackMember(UpdateStackMember::EXECUTE_FRAME,framesThatNeedUpdating[i]));
		}
	}
	
	return false;
}

bool TimeEngine::executeFrameUpdateStack()
{
    // stores the arrival data for the updated frames in order for a single executeFrameUpdateStack()
	//vector<ObjectList*> frameArrivalAtUpdate;

	while (frameUpdateStack.size() > 0)
	{
		const UpdateStackMember nextUpdate(frameUpdateStack.back());
		frameUpdateStack.pop_back();

		if (nextUpdate.getType() == UpdateStackMember::CLEAR_PARADOX)
		{
			previousArrival[nextUpdate.getFrame()].clear();
		}
		else if (nextUpdate.getType() == UpdateStackMember::EXECUTE_FRAME)
		{
            if (updateFrame(nextUpdate.getFrame())) {
                return true;
            }
		}
	}
    return false;
}

boost::shared_ptr<ObjectList> TimeEngine::getLastArrival(int time)
{
	return lastArrival[time];
}