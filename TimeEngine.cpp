#include "TimeEngine.h"

#include <vector>
using namespace std;

TimeEngine::TimeEngine(int newTimeLength, vector<vector<bool> > wallmap, int newWallSize, int newGravity)
{
	playerGuyIndex = -1;
	currentPlayerFrame = -1;

	timeLineLength = newTimeLength;

	arrivalFrames = timeLineLength;
	departuresFrames = timeLineLength+1; // permanentDeparture

	permanentDepatureIndex = departuresFrames-1;

	arrivalDeparturePair = boost::shared_ptr<ArrivalDepartureMap>(new ArrivalDepartureMap(timeLineLength));

	lastArrival.reserve(arrivalFrames);
	previousArrival.reserve(arrivalFrames);

	for (int i = 0; i < arrivalFrames; ++i)
	{
		lastArrival.push_back(boost::shared_ptr<ObjectList> (new ObjectList()));
		previousArrival.push_back(vector<boost::shared_ptr<ObjectList> >());
	}

	physics = boost::shared_ptr<PhysicsEngine>(new PhysicsEngine(timeLineLength, wallmap, newWallSize, newGravity));

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
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, 0)));
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, timeLineLength-1)));
			if (executeFrameUpdateStack())
			{
				return false;
			}

			// save gamestate
			boost::shared_ptr<ArrivalDepartureMap> tempArrivalDeparturePair;
			tempArrivalDeparturePair = arrivalDeparturePair;

			arrivalDeparturePair = boost::shared_ptr<ArrivalDepartureMap>(new ArrivalDepartureMap(timeLineLength));

			// propgate from end of time first
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, timeLineLength-1)));
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, 0)));
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
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, 0)));
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
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, timeLineLength-1)));
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

vector<boost::shared_ptr<ObjectList> > TimeEngine::getNextPlayerFrame(boost::shared_ptr<InputList> newInputData)
{

	playerInput.push_back(newInputData);
	++playerGuyIndex;

	currentPlayerFrame = nextPlayerFrame;

	frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1, currentPlayerFrame)));

	if (executeFrameUpdateStack()) // paradox occured
	{
		return frameArrivalAtUpdate;
	}
	else // paradox did not occur
	{
		return vector<boost::shared_ptr<ObjectList> >(1,currentPlayerFrameData);
	}
}

bool TimeEngine::updateFrame(int frame)
{
	// get the arrivals for the physics frame by going across a row 
	boost::shared_ptr<ObjectList> arrivals = arrivalDeparturePair->getArrivals(frame, playerGuyIndex+1);

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
	boost::shared_ptr<TimeObjectListList> departures = physics->executeFrame(*arrivals.get(), frame, playerGuyIndex, playerInput);

	if (frame == currentPlayerFrame)
	{
		updateStartFirst = (physics->getPlayerDirection() == 1);
		nextPlayerFrame = physics->getNextPlayerFrame();
		currentPlayerFrameData = boost::shared_ptr<ObjectList>(arrivals);
	}
	

	// update departures from this frame
	departures->sortObjectLists();
	vector<int> framesThatNeedUpdating = arrivalDeparturePair->updateDeparturesFromTime(frame, departures);

	// add frames that need updating to execution stack
	if (framesThatNeedUpdating.size() > 0)
	{
		previousArrival[frame].push_back(lastArrival[frame]);
		frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(0,frame)));
		// the bottom added frame must remember that it will clear paradox check data from parent frame if it finishes execution
	}

	if (updateStartFirst)
	{
		for (int i = framesThatNeedUpdating.size()-1; i >= 0 ; --i)
		{
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1,framesThatNeedUpdating[i])));
		}
	}
	else
	{
		for (unsigned int i = 0; i < framesThatNeedUpdating.size(); ++i)
		{
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1,framesThatNeedUpdating[i])));
		}
	}
	
	return false;
}

bool TimeEngine::executeFrameUpdateStack()
{
	frameArrivalAtUpdate.clear();

	while (frameUpdateStack.size() > 0)
	{
		int frame = frameUpdateStack.back()->getFrame();
		int type = frameUpdateStack.back()->getType();

		frameUpdateStack.pop_back();

		if (type == 0) // clear paradox
		{
			previousArrival[frame].clear();
		}
		else if (type == 1) // execute frame
		{
			if (updateFrame(frame))
			{
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