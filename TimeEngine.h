#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H

#include "WorldState.h"
#include "FrameUpdateSet.h"
#include "ProgressMonitor.h"

#include <boost/container/vector.hpp>
#include <boost/swap.hpp>

#include "InputList_fwd.h"
#include "Level.h"
#include "Frame.h"

#include <boost/move/move.hpp>
#include <boost/swap.hpp>
#include <utility>
namespace hg {
//TimeEngines are movable but non-copyable.
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
                updatedFrames_(hg::move(updatedFrames))
        {}
        Frame const* currentPlayerFrame() const {
            return currentPlayerFrame_;
        }
        Frame const* nextPlayerFrame() const {
            return nextPlayerFrame_;
        }
        FrameListList const& updatedFrames() const {
            return updatedFrames_;
        }
        Frame const* currentPlayerFrame_;
        Frame const* nextPlayerFrame_;
        FrameListList updatedFrames_;
    };
    TimeEngine(BOOST_RV_REF(TimeEngine) other) :
        speedOfTime_(boost::move(other.speedOfTime_)),
        worldState_(boost::move(other.worldState_)),
        wall_(boost::move(other.wall_))
    {}
    TimeEngine& operator=(BOOST_RV_REF(TimeEngine) other)
    {
        speedOfTime_ = boost::move(other.speedOfTime_);
        worldState_ = boost::move(other.worldState_);
        wall_ = boost::move(other.wall_);
        return *this;
    }
    /**
     * Constructs a new TimeEngine with the given Level
     *
     * Propagates the level to fully initialise the TimeEngine
     * (objects begin at all points in time throughout the level,
     * and so must be propagated through from the start and the end)
     * Throws InvalidLevelException if level is not correct
     * A correct level has exactly one guy.
     * Exception Safety: Strong
     */
    explicit TimeEngine(Level&& level/*, ProgressMonitor& monitor*/);

    void swap(TimeEngine& other);

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
    std::vector<InputList> const& getReplayData() const;
    
    Wall const& getWall() const { return wall_; }
    
    std::size_t getTimelineLength() const { return worldState_.getTimelineLength(); }
private:
    unsigned int speedOfTime_;
    //state of world at end of last executed frame
    WorldState worldState_;
    //Wall duplicated here, it is also in physics.
    //This may not be ideal, but it simplifies a few things.
    //No, this is probably plain silly. Please fix/justify.
    Wall wall_;
    
    BOOST_MOVABLE_BUT_NOT_COPYABLE(TimeEngine)
};
}
#endif //HG_TIME_ENGINE_H
