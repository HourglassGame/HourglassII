#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H

#include "PhysicsEngine.h"
#include "WorldState.h"
#include "TimelineState.h"

#include <vector>
#include <boost/swap.hpp>

namespace hg {
class InputList;
class ObjectList;
class TimeObjectListList;
class ParadoxException;
class Level;
class Frame;
class TimeEngine
{
public:
    typedef std::vector<FrameUpdateSet> FrameListList;
    struct RunResult
    {
        Frame* currentPlayerFrame() {
            return currentPlayerFrame_;
        }
        Frame* nextPlayerFrame() {
            return nextPlayerFrame_;
        }
        const FrameListList& updatedFrames() {
            return *updatedFrames_;
        }
        void swap(RunResult& other)
        {
            boost::swap(currentPlayerFrame_, other.currentPlayerFrame_);
            boost::swap(nextPlayerFrame_, other.nextPlayerFrame_);
            boost::swap(updatedFrames_, other.updatedFrames_);
        }
private:
        friend class TimeEngine;
        Frame* currentPlayerFrame_;
        Frame* nextPlayerFrame_;
        const FrameListList* updatedFrames_;
    };

    /**
     * Constructs a new TimeEngine with the given Level
     *
     * Propagates the level to fully initialise the TimeEngine
     * (objects begin at all points in time throughout the level,
     * and so must be propagated through from the start and the end)
     * Throws InvalidLevelException if level is not correct
     * A correct level has exacty one guy.
     */
    TimeEngine(const Level& level);


    /**
     * Takes the new input data and uses that to update the state of the world and returns the current player frame
     * and a list of the frames which were updated in each propagation round. The current player frame is the last
     * in which the player had input.
     */
    RunResult runToNextPlayerFrame(const InputList& newInputData);

    /**
     * Returns a pointer to the frame in the TimeEngine which corresponds to whichFrame
     */
    Frame* getFrame(const FrameID& whichFrame);
    std::vector<InputList> getReplayData() const;
private:
    unsigned int speedOfTime;
    //state of world at end of last executed frame
    WorldState worldState;
    //Stores the lists of frames which were updated for each step of the execution.
    //This is a member of TimeEngine for performance reasons alone;
    //(logically a new one is created with each call to runToNextPlayerFrame).
    FrameListList updatedList;
};
}
#endif //HG_TIME_ENGINE_H
