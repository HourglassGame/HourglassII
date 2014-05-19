#ifndef HG_TIME_ENGINE_H
#define HG_TIME_ENGINE_H

#include "WorldState.h"
#include "FrameUpdateSet.h"

#include <boost/container/vector.hpp>
#include <boost/swap.hpp>

#include "InputList_fwd.h"
#include "Level.h"
#include "Frame.h"
#include "as_lvalue.h"

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
class Frame;
class Level;
class TimeEngineImpl;
class TimeEngine
{
public:
    typedef boost::container::vector<FrameUpdateSet> FrameListList;
    struct RunResult
    {
        Frame const *currentPlayerFrame;
        Frame const *nextPlayerFrame;
        FrameListList updatedFrames;
    };
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
    explicit TimeEngine(
        Level &&level,
        OperationInterrupter &interrupter = as_lvalue(NullOperationInterrupter()));

    TimeEngine(TimeEngine const &);
    TimeEngine(TimeEngine &&) noexcept;
    TimeEngine &operator=(TimeEngine const&);
    TimeEngine &operator=(TimeEngine &&) noexcept;
    ~TimeEngine() noexcept;
    void swap(TimeEngine &o) noexcept;

    /**
     * Takes the new input data and uses that to update the state of the world and returns the current player frame
     * and a list of the frames which were updated in each propagation round. The current player frame is the last
     * in which the player had input.
     * Exception Safety: Weak
     */
    RunResult runToNextPlayerFrame(
        InputList const &newInputData,
        OperationInterrupter &interrupter = as_lvalue(NullOperationInterrupter()));

    /**
     * Returns a pointer to the frame in the TimeEngine which corresponds to whichFrame
     */
    Frame const *getFrame(FrameID const &whichFrame) const noexcept;// { return worldState.getFrame(whichFrame); }
    std::vector<InputList> const &getReplayData() const noexcept;// { return worldState.getReplayData(); }
    Wall const &getWall() const noexcept;// { return wall; }
    int getTimelineLength() const noexcept;// { return worldState.getTimelineLength(); }
private:
    clone_ptr<TimeEngineImpl> impl;
};
}
#endif //HG_TIME_ENGINE_H
