#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H

#include "WorldState.h"
#include "FrameUpdateSet.h"

#include <vector>
#include <boost/swap.hpp>

#include "InputList_fwd.h"
#include "Level_fwd.h"
#include "Frame_fwd.h"
#include <boost/move/move.hpp>
#include <boost/container/vector.hpp>
namespace hg {
//TimeEngines are moveable but noncopyable
//This is not due to any underlying limitation, but simply
//because the use of Frame pointers makes
//writing a correct copy assignment operator or constructor
//a rather involved task.

//If the capability to copy time engines becomes important then
//copying operations can be added.
class TimeEngine
{
public:
    typedef boost::container::vector<FrameUpdateSet> FrameListList;
    struct RunResult
    {
        RunResult(
            Frame const* currentPlayerFrame,
            Frame const* nextPlayerFrame,
            BOOST_RV_REF(FrameListList) updatedFrames) :
                currentPlayerFrame_(currentPlayerFrame),
                nextPlayerFrame_(nextPlayerFrame),
                updatedFrames_(updatedFrames)
        {
        }
        Frame const* currentPlayerFrame() const {
            return currentPlayerFrame_;
        }
        Frame const* nextPlayerFrame() const {
            return nextPlayerFrame_;
        }
        FrameListList const& updatedFrames() const {
            return updatedFrames_;
        }
    private:
        Frame const* currentPlayerFrame_;
        Frame const* nextPlayerFrame_;
        FrameListList updatedFrames_;
    };

    /**
     * Constructs a new TimeEngine with the given Level
     *
     * Propagates the level to fully initialise the TimeEngine
     * (objects begin at all points in time throughout the level,
     * and so must be propagated through from the start and the end)
     * Throws InvalidLevelException if level is not correct
     * A correct level has exacty one guy.
     * Exception Safety: Strong
     */
    explicit TimeEngine(Level const& level);


    TimeEngine(BOOST_RV_REF(TimeEngine) o);
    TimeEngine& operator=(BOOST_RV_REF(TimeEngine) o);

    /**
     * Takes the new input data and uses that to update the state of the world and returns the current player frame
     * and a list of the frames which were updated in each propagation round. The current player frame is the last
     * in which the player had input.
     * Exception Safety: Weak
     */
    RunResult runToNextPlayerFrame(InputList const& newInputData);

    /**
     * Returns a pointer to the frame in the TimeEngine which corresponds to whichFrame
     * Non-const only so a non-const Frame* can be returned.
     * Please investigate this constness further!
     * Exception Safety: No Throw
     */
    Frame* getFrame(FrameID const& whichFrame);
    // Exception Safety: Strong
    std::vector<InputList> getReplayData() const;
private:
    unsigned int speedOfTime;
    //state of world at end of last executed frame
    WorldState worldState;
    
    BOOST_MOVABLE_BUT_NOT_COPYABLE(TimeEngine)
};
}
#endif //HG_TIME_ENGINE_H
