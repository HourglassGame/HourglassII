#include "TimeEngine.h"

#include <vector>
using namespace std;

TimeEngine::TimeEngine(int newFrameCount)
{
	playerGuyIndex = -1;

	frameCount = newFrameCount;

	for (int i = 0; i < newFrameCount; ++i)
	{
		arrivalDeparturePair.push_back( vector<boost::shared_ptr<ObjectList>>());
		for (int j = 0; j < newFrameCount; ++j)
		{
			arrivalDeparturePair[i].push_back(boost::shared_ptr<ObjectList> (new ObjectList()));
		
		}
	}

	for (int i = 0; i < newFrameCount; ++i)
	{
		previousArrival.push_back(vector<boost::shared_ptr<ObjectList>>());
	}

	physics = boost::shared_ptr<PhysicsEngine>(new PhysicsEngine());

}

boost::shared_ptr<ObjectList> TimeEngine::getNextPlayerFrame(boost::shared_ptr<InputList> newInputData)
{

	playerInput.push_back(newInputData);
	++playerGuyIndex;

	currentPlayerFrame = nextPlayerFrame;

	frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(currentPlayerFrame, -1)));

	executeFrameUpdateStack();

	return currentPlayerFrameData;
}

void TimeEngine::updateFrame(int frame)
{
	// get the arrivals for the physics frame by going across a row 
	boost::shared_ptr<ObjectList> arrivals(new ObjectList);
	for (int i = 0; i < frameCount; ++i)
	{
		arrivals->add(arrivalDeparturePair[frame][i], playerGuyIndex+1);
	}
	arrivals->sortElements(); // to ensure determinism of input

	for (unsigned int i = 0; i < previousArrival[frame].size(); ++i)
	{
		if (previousArrival[frame][i]->equals(arrivals))
		{
			// a paradox has occured
		}
	}

	previousArrival[frame].push_back(arrivals);

	// get departures for the frame
	vector<boost::shared_ptr<ObjectList>> departures = physics->executeFrame(arrivals, frame, frameCount, playerGuyIndex, playerInput);

	if (frame == currentPlayerFrame)
	{
		nextPlayerFrame = physics->getNextPlayerFrame();
		currentPlayerFrameData = arrivals;
	}
	
	// update departures from this frame
	vector<int> framesThatNeedUpdating;

	for (int i = 0; i < frameCount; ++i)
	{
		departures[i]->sortElements();
		if (!(departures[i]->equals(arrivalDeparturePair[i][frame])) )
		{
			framesThatNeedUpdating.push_back(i);
			arrivalDeparturePair[i][frame] == departures[i];
		}
	}

	// add frames that need updating to execution stack
	if (framesThatNeedUpdating.size() > 0)
	{
		frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(0,frame)));
		// the bottom added frame must remember that it will clear paradox check data from parent frame if it finishes execution
	}

	for (unsigned int i = 0; i < framesThatNeedUpdating.size(); ++i)
	{
		frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1,framesThatNeedUpdating[i])));
	}
}

void TimeEngine::executeFrameUpdateStack()
{
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
			updateFrame(frame);
		}
		
	}
	
}