#include "TimeEngine.h"

#include <vector>
using namespace std;

TimeEngine::TimeEngine(int newTimeLength, vector<vector<bool>> wallmap)
{
	playerGuyIndex = -1;

	timeLineLength = newTimeLength;

	arrivalFrames = timeLineLength;
	departuresFrames = timeLineLength+1; // permanentDeparture

	permanentDepatureIndex = departuresFrames-1;

	for (int i = 0; i < arrivalFrames; ++i)
	{
		arrivalDeparturePair.push_back( vector<boost::shared_ptr<ObjectList>>());
		for (int j = 0; j < departuresFrames; ++j)
		{
			arrivalDeparturePair[i].push_back(boost::shared_ptr<ObjectList> (new ObjectList()));
		}
	}

	for (int i = 0; i < timeLineLength; ++i)
	{
		previousArrival.push_back(vector<boost::shared_ptr<ObjectList>>());
	}

	physics = boost::shared_ptr<PhysicsEngine>(new PhysicsEngine(timeLineLength, playerInput, wallmap));

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
			arrivalDeparturePair[timeLineLength-1][permanentDepatureIndex]->addBox(initialObjects->getBoxList()[i]);
		}
		else
		{
			atEnd = true;
			arrivalDeparturePair[0][permanentDepatureIndex]->addBox(initialObjects->getBoxList()[i]);
		}
		
	}

	// guys
	if (initialObjects->getGuyList().size() == 1)
	{
		arrivalDeparturePair[guyStartTime][permanentDepatureIndex]->addGuy(initialObjects->getGuyList()[0]);
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
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(0, 1)));
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(timeLineLength-1, 1)));
			if (executeFrameUpdateStack())
			{
				return false;
			}

			// save gamestate
			vector<vector<boost::shared_ptr<ObjectList>>> tempArrivalDeparturePair;

			for (int i = 0; i < arrivalFrames; ++i)
			{
				tempArrivalDeparturePair.push_back( vector<boost::shared_ptr<ObjectList>>());
				for (int j = 0; j < departuresFrames; ++j)
				{
					tempArrivalDeparturePair[i].push_back(boost::shared_ptr<ObjectList> (arrivalDeparturePair[i][j]));
					arrivalDeparturePair[i][j] = boost::shared_ptr<ObjectList>(new ObjectList());
				}
			}

			// propgate from end of time first
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(timeLineLength-1, 1)));
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(0, 1)));
			if (executeFrameUpdateStack())
			{
				return false;
			}

			// compare start first with end first
			for (int i = 0; i < arrivalFrames; ++i)
			{
				for (int j = 0; j < departuresFrames; ++j)
				{
					if (!(arrivalDeparturePair[i][j]->equals(tempArrivalDeparturePair[i][j])))
					{
						return false;
					}
				}
			}


		}
		else // only the beginning require propagation
		{
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(0, 1)));
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
			frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(timeLineLength-1, 1)));
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

boost::shared_ptr<ObjectList> TimeEngine::getNextPlayerFrame(boost::shared_ptr<InputList> newInputData)
{

	playerInput.push_back(newInputData);
	++playerGuyIndex;

	currentPlayerFrame = nextPlayerFrame;

	frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(currentPlayerFrame, 1)));

	executeFrameUpdateStack();

	return currentPlayerFrameData;
}

bool TimeEngine::updateFrame(int frame)
{
	// get the arrivals for the physics frame by going across a row 
	boost::shared_ptr<ObjectList> arrivals(new ObjectList);
	for (int i = 0; i < departuresFrames; ++i)
	{
		arrivals->add(arrivalDeparturePair[frame][i], playerGuyIndex+1);
	}
	arrivals->sortElements(); // to ensure determinism of input

	for (unsigned int i = 0; i < previousArrival[frame].size(); ++i)
	{
		if (previousArrival[frame][i]->equals(arrivals))
		{
			return true;
		}
	}

	// get departures for the frame
	vector<boost::shared_ptr<ObjectList>> departures = physics->executeFrame(arrivals, frame, playerGuyIndex);

	if (frame == currentPlayerFrame)
	{
		updateStartFirst = (physics->getPlayerDirection() == 1);
		nextPlayerFrame = physics->getNextPlayerFrame();
		currentPlayerFrameData = arrivals;
	}
	
	// update departures from this frame
	vector<int> framesThatNeedUpdating;

	if (updateStartFirst)
	{
		for (int i = arrivalFrames-1; i >= 0; --i)
		{
			departures[i]->sortElements();
			if (!(departures[i]->equals(arrivalDeparturePair[i][frame])) )
			{
				framesThatNeedUpdating.push_back(i);
				arrivalDeparturePair[i][frame] == departures[i];
			}
		}
		
	}
	else
	{
		for (int i = 0; i < arrivalFrames; ++i)
		{
			departures[i]->sortElements();
			if (!(departures[i]->equals(arrivalDeparturePair[i][frame])) )
			{
				framesThatNeedUpdating.push_back(i);
				arrivalDeparturePair[i][frame] == departures[i];
			}
		}
	}

	// add frames that need updating to execution stack
	if (framesThatNeedUpdating.size() > 0)
	{
		previousArrival[frame].push_back(arrivals);
		frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(0,frame)));
		// the bottom added frame must remember that it will clear paradox check data from parent frame if it finishes execution
	}

	for (unsigned int i = 0; i < framesThatNeedUpdating.size(); ++i)
	{
		frameUpdateStack.push_back(boost::shared_ptr<UpdateStackMember>(new UpdateStackMember(1,framesThatNeedUpdating[i])));
	}

	return false;
}

bool TimeEngine::executeFrameUpdateStack()
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
			if (updateFrame(frame))
			{
				return true;
			}
		}
	}

	return false;
}