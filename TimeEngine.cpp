#include <iostream>
#include <cassert>
#include <iterator>

#include "TimeEngine.h"

#include "InvalidLevelException.h"
#include "ParadoxException.h"
#include "TotalState.h"

#include <boost/foreach.hpp>
#define reverse_foreach BOOST_REVERSE_FOREACH

using namespace ::std;
using namespace ::hg;

TimeEngine::TimeEngine(unsigned int timeLineLength,
                       vector<vector<bool> > wallmap,
                       int newWallSize,
                       int newGravity,
                       const ObjectList& initialObjects,
                       unsigned int guyStartTime) :
endOfFrameState(ArrivalDepartureMap(timeLineLength),true,timeLineLength,guyStartTime),
playerInput(),
physics(timeLineLength, wallmap, newWallSize, newGravity)
{
    assert(timeLineLength > 0);
    // boxes
    for (vector<Box>::const_iterator it(initialObjects.getBoxListRef().begin()),
         end(initialObjects.getBoxListRef().end()); it != end; ++it)
    {
        if (it->getTimeDirection() == FORWARDS)
        {
            endOfFrameState.arrivalDepartures.permanentDepartureObjectList(0).addBox(*it);
        }
        else
        {
            endOfFrameState.arrivalDepartures.permanentDepartureObjectList(timeLineLength-1).addBox(*it);
        }
    }
    
    // guys
    endOfFrameState.currentPlayerFrame = timeLineLength;
    endOfFrameState.nextPlayerFrame = guyStartTime;
    if (initialObjects.getGuyListRef().size() != 1)
    {
        throw InvalidLevelException();
    }
    
    //** determine level
    
    // propgate from start of time first
    vector<unsigned int> startFirstFrameUpdateStack;
    startFirstFrameUpdateStack.push_back(0);
    startFirstFrameUpdateStack.push_back(timeLineLength-1);
    
    WorldState startFirstState(executeFrameUpdateStackNoParadoxCheck(endOfFrameState, startFirstFrameUpdateStack));
    
    // propgate from end of time first
    vector<unsigned int> endFirstFrameUpdateStack;
    endFirstFrameUpdateStack.push_back(timeLineLength-1);
    endFirstFrameUpdateStack.push_back(0);
    
    WorldState endFirstState(executeFrameUpdateStackNoParadoxCheck(endOfFrameState, endFirstFrameUpdateStack));
    
    // compare start first with end first
    if (startFirstState == endFirstState)
    {
        endOfFrameState = startFirstState;
        endOfFrameState.arrivalDepartures.permanentDepartureObjectList(guyStartTime).addGuy(initialObjects.getGuyListRef()[0]);
        return;
    }
    throw InvalidLevelException();
}

const ObjectList TimeEngine::getNextPlayerFrame(const InputList& newInputData)
{
    playerInput.push_back(newInputData);
    endOfFrameState.currentPlayerFrame = endOfFrameState.nextPlayerFrame;
    endOfFrameState = executeFrameUpdateStack(endOfFrameState, vector<unsigned int>(1,endOfFrameState.currentPlayerFrame));
    //Get arrivals at frame where player just arrived.
    return endOfFrameState.arrivalDepartures.getArrivals(endOfFrameState.nextPlayerFrame);
}


WorldState TimeEngine::executeFrameUpdateStackNoParadoxCheck(WorldState currentState, 
                                                               vector<unsigned int> frameUpdateStack) const
{
    while (frameUpdateStack.size() > 0)
    {
        const unsigned int nextUpdate(frameUpdateStack.back());
        frameUpdateStack.pop_back();
        updateFrame(nextUpdate, frameUpdateStack, currentState);
    }
    return currentState;
}

WorldState TimeEngine::executeFrameUpdateStack(WorldState currentState, vector<unsigned int> frameUpdateStack) const
{
    //world state hashes in order for a single executeFrameUpdateStack()
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
                    {
                        //This is where executeFrameUpdateStack is called many times and the results compared
                        //This bit could easily be done in parallel
                        vector<unsigned int> newFrameUpdateStack(it->stackState);
                        newFrameUpdateStack.pop_back();
                        if (newFrameUpdateStack.empty()) {
                            throw ParadoxException();
                        }
                        vector<WorldState> alternateStates;
                        while (it!=end) {
                            WorldState possibleState(executeFrameUpdateStack(it->worldState, newFrameUpdateStack));
                            for (vector<WorldState>::iterator stateit(alternateStates.begin()),
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
            }
            previousStates.push_back(newState);
        }
        
        const unsigned int nextUpdate(frameUpdateStack.back());
        frameUpdateStack.pop_back();
        updateFrame(nextUpdate, frameUpdateStack, currentState);
    }
    return currentState;
}


void TimeEngine::updateFrame(unsigned int frame, vector<unsigned int>& frameUpdateStack, WorldState& currentState) const
{
    // get the arrivals for the physics frame by going across a row
    const ObjectList arrivals(currentState.arrivalDepartures.getArrivals(frame));
    
    if (frame == currentState.currentPlayerFrame)
    {
        assert(arrivals.getGuyListRef().rbegin() != arrivals.getGuyListRef().rend());
        assert((*(arrivals.getGuyListRef().rbegin())).getRelativeIndex() == playerInput.size() - 1
               && "arrivals is sorted so last guy should be player guy");
        currentState.updateStartFirst = ((*(arrivals.getGuyListRef().rbegin())).getTimeDirection() == FORWARDS);
    }
    
    // get departures for the frame, update currentPlayerFrame and nextPlayerFrame
    // if appropriate
    TimeObjectListList departures(physics.executeFrame(arrivals, frame, playerInput, 
                                                       currentState.currentPlayerFrame,
                                                       currentState.nextPlayerFrame));
    
    // update departures from this frame
    departures.sortObjectLists();
    const vector<unsigned int> framesThatNeedUpdating(currentState.arrivalDepartures.updateDeparturesFromTime(frame, departures));
    
    if (currentState.updateStartFirst)
    {
        frameUpdateStack.insert(frameUpdateStack.end(),framesThatNeedUpdating.rbegin(),framesThatNeedUpdating.rend());
    }
    else
    {
        frameUpdateStack.insert(frameUpdateStack.end(),framesThatNeedUpdating.begin(),framesThatNeedUpdating.end());
    }
}
