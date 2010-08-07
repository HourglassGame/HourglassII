#include "TimeEngine.h"

#include "ParadoxException.h"
#include "TotalState.h"

#include <vector>
#include <iostream>
#include <cassert>
using namespace std;
using namespace hg;

TimeEngine::TimeEngine(int newTimeLength, vector<vector<bool> > wallmap, int newWallSize, int newGravity) :
timeLineLength(newTimeLength),
endOfFrameState(ArrivalDepartureMap(newTimeLength),true,-1,-1),
physics(newTimeLength, wallmap, newWallSize, newGravity)
{
}

// returns if the level creator not a foo
bool TimeEngine::checkConstistencyAndPropagateLevel(const ObjectList& initialObjects, int guyStartTime)
{
	// boxes
	for (unsigned int i = 0; i < initialObjects.getBoxListRef().size(); ++i)
	{
		if (initialObjects.getBoxListRef()[i].getTimeDirection() == FORWARDS)
		{
			endOfFrameState.arrivalDepartures.permanentDepartureObjectList(0).addBox(initialObjects.getBoxListRef()[i]);
		}
		else
		{
			endOfFrameState.arrivalDepartures.permanentDepartureObjectList(timeLineLength-1).addBox(initialObjects.getBoxListRef()[i]);
		}
	}
    
	// guys
    endOfFrameState.currentPlayerFrame = -1;
	endOfFrameState.nextPlayerFrame = guyStartTime;
	if (initialObjects.getGuyListRef().size() == 1)
	{
		endOfFrameState.arrivalDepartures.permanentDepartureObjectList(guyStartTime).addGuy(initialObjects.getGuyListRef()[0]);
	}
	else
	{
		return false; // must have exactly 1 guy in level
	}

	//** determine level
    try
    {
        // propgate from start of time first
        std::vector<int> startFirstFrameUpdateStack;
        startFirstFrameUpdateStack.push_back(0);
        startFirstFrameUpdateStack.push_back(timeLineLength-1);

        WorldState startFirstState(executeFrameUpdateStack(endOfFrameState, startFirstFrameUpdateStack));

        // propgate from end of time first
        std::vector<int> endFirstFrameUpdateStack;
        endFirstFrameUpdateStack.push_back(timeLineLength-1);
        endFirstFrameUpdateStack.push_back(0);

        WorldState endFirstState(executeFrameUpdateStack(endOfFrameState, endFirstFrameUpdateStack));

        // compare start first with end first
        if (startFirstState == endFirstState)
        {
            endOfFrameState = startFirstState;
            return true;
        }
    }
    catch (ParadoxException&)
    {
    }
    return false;
}

ObjectList TimeEngine::getNextPlayerFrame(const InputList& newInputData)
{
    playerInput.push_back(newInputData);
    endOfFrameState.currentPlayerFrame = endOfFrameState.nextPlayerFrame;
    endOfFrameState = executeFrameUpdateStack(endOfFrameState, std::vector<int>(1,endOfFrameState.currentPlayerFrame));
    //Get arrivals at frame where player just arrived. (`playerInput.size()+1' to include player guy)
    //This is slightly different from before, but I think it makes more sense
    return endOfFrameState.arrivalDepartures.getArrivals(endOfFrameState.nextPlayerFrame, playerInput.size()+1);
}

WorldState TimeEngine::executeFrameUpdateStack(WorldState currentState, 
                                               std::vector<int> frameUpdateStack) const
{
    //world state in order for a single executeFrameUpdateStack()
	vector<TotalState> previousStates;
    
	while (frameUpdateStack.size() > 0)
	{
        
        {
            TotalState newState(currentState, frameUpdateStack);
            
            //If the whole world is in the same state and the same
            //frame is to be updated next then it is a local paradox
            //To test whether global paradox need to pop out of stack
            //at point of first instance and check whether the global 
            //state depends on which of the local states is assumed to be true.
            //If it does then it is a paradox
            for (vector<TotalState>::const_iterator it(previousStates.begin()),
                 end(previousStates.end());
                 it != end; ++it) {
                if (*it == newState) {
                    //This is where executeFrameUpdateStack is called many times and the results compared
                    //This bit could easily be done in parallel
                    std::vector<int> newFrameUpdateStack(it->stackState);
                    newFrameUpdateStack.pop_back();
                    std::vector<WorldState> alternateStates;
                    while (it!=end) {
                        WorldState possibleState(executeFrameUpdateStack(it->worldState, newFrameUpdateStack));
                        for (std::vector<WorldState>::iterator stateit(alternateStates.begin()),
                            stateend(alternateStates.end()); stateit != stateend; ++stateit) {
                            if (possibleState != *stateit) {
                                throw ParadoxException();
                            }
                        }
                        alternateStates.push_back(possibleState);
                        ++it;
                    }
                    //Any index would do as they are all the same!
                    return alternateStates[0];
                }
            }
            previousStates.push_back(newState);
        }
        
        const int nextUpdate(frameUpdateStack.back());
        frameUpdateStack.pop_back();
        updateFrame(nextUpdate, frameUpdateStack, currentState);
    }
    return currentState;
}

void TimeEngine::updateFrame(int frame, std::vector<int>& frameUpdateStack, WorldState& currentState) const
{
	// get the arrivals for the physics frame by going across a row
	ObjectList arrivals(currentState.arrivalDepartures.getArrivals(frame, playerInput.size()));
    
    if (frame == currentState.currentPlayerFrame)
	{
        assert(arrivals.getGuyListRef().rbegin() != arrivals.getGuyListRef().rend());
        assert((*(arrivals.getGuyListRef().rbegin())).getRelativeIndex() == playerInput.size()-1
               && "arrivals is sorted so last guy should be player guy");
        currentState.updateStartFirst = ((*(arrivals.getGuyListRef().rbegin())).getTimeDirection() == FORWARDS);
	}
    
	// get departures for the frame, update currentPlayerFrame and nextPlayerFrame if needed
	TimeObjectListList departures(physics.executeFrame(arrivals, frame, playerInput, 
                                                       currentState.currentPlayerFrame, currentState.nextPlayerFrame));
    
	// update departures from this frame
	departures.sortObjectLists();
	vector<int> framesThatNeedUpdating(currentState.arrivalDepartures.updateDeparturesFromTime(frame, departures));
    
	if (currentState.updateStartFirst)
	{
		for (vector<int>::const_reverse_iterator it(framesThatNeedUpdating.rbegin()), 
             end(framesThatNeedUpdating.rend()); it != end; ++it)
		{
			frameUpdateStack.push_back(*it);
		}
	}
	else
	{
		for (vector<int>::const_iterator it(framesThatNeedUpdating.begin()), 
             end(framesThatNeedUpdating.end()); it != end; ++it)
		{
			frameUpdateStack.push_back(*it);
		}
	}
}