#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"

#include <boost/swap.hpp>

namespace hg {
struct TimeEngineImpl {
    TimeEngineImpl *clone() const {
        return new TimeEngineImpl(*this);
    }
    unsigned int speedOfTime;
    //state of world at end of last executed frame
    WorldState worldState;
    //Wall duplicated here, it is also in physics.
    //This may not be ideal, but it simplifies a few things.
    //No, this is probably plain silly. Please fix/justify.
    Wall wall;
};

TimeEngine::TimeEngine(Level &&level, OperationInterrupter &interrupter) :
    impl(
      new TimeEngineImpl{
        level.speedOfTime,
        WorldState(
            level.timelineLength,
            std::move(level.initialGuy),
            std::move(level.guyStartTime),
            PhysicsEngine(Environment(level.environment), std::move(level.triggerSystem)),
            std::move(level.initialObjects),
            interrupter),
        level.environment.wall})
{
}

void TimeEngine::swap(TimeEngine &o) noexcept {
    std::swap(impl, o.impl);
}

TimeEngine::RunResult
TimeEngine::runToNextPlayerFrame(InputList const &newInputData, OperationInterrupter &interrupter)
{
    impl->worldState.addNewInputData(newInputData);
    FrameListList updatedList;
    updatedList.reserve(impl->speedOfTime);
    for (unsigned int i(0); i < impl->speedOfTime && !interrupter.interrupted(); ++i) {
        updatedList.push_back(impl->worldState.executeWorld(interrupter));
    }
    return RunResult{
        impl->worldState.getCurrentPlayerFrame(),
        impl->worldState.getNextPlayerFrame(),
        std::move(updatedList)};
}

Frame const *TimeEngine::getFrame(FrameID const &whichFrame) const noexcept { return impl->worldState.getFrame(whichFrame); }
std::vector<InputList> const &TimeEngine::getReplayData() const noexcept { return impl->worldState.getReplayData(); }
Wall const &TimeEngine::getWall() const noexcept { return impl->wall; }
int TimeEngine::getTimelineLength() const noexcept { return impl->worldState.getTimelineLength(); }


TimeEngine::TimeEngine(TimeEngine const &) = default;
TimeEngine::TimeEngine(TimeEngine &&) noexcept = default;
TimeEngine &TimeEngine::operator=(TimeEngine const&) = default;
TimeEngine &TimeEngine::operator=(TimeEngine &&) noexcept = default;
TimeEngine::~TimeEngine() noexcept = default;


}//namespace hg
