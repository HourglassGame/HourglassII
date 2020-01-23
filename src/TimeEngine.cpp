#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"
#include "GlobalConst.h"

#include <boost/swap.hpp>
//#include <iostream>

namespace hg {
struct TimeEngineImpl final {
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
        level.environment.wall}),
    paradoxPressure(0),
    paradoxPressureDecay(0),
    guyDirection(TimeDirection::INVALID),
    guyFrameNumber(0)
{
}

void TimeEngine::swap(TimeEngine &o) noexcept {
    std::swap(impl, o.impl);
}

hg::GuyOutputInfo const &findCurrentGuy(mt::std::vector<GuyOutputInfo> const &guyRange, std::size_t index)
{
    for (GuyOutputInfo const &guyInfo : guyRange)
    {
        if (guyInfo.getIndex() == index)
        {
            return guyInfo;
        }
    }
}

TimeEngine::RunResult TimeEngine::runToNextPlayerFrame(InputList const &newInputData, size_t const relativeGuyIndex, OperationInterrupter &interrupter)
{
    impl->worldState.addNewInputData(newInputData);
    FrameListList updatedList;
    updatedList.reserve(impl->speedOfTime);
    for (unsigned int i(0); i < impl->speedOfTime && !interrupter.interrupted(); ++i) {
        updatedList.push_back(impl->worldState.executeWorld(interrupter));
    }

    // Only generate paradox pressure for waves behind the current guy
    std::size_t guyIndex = getGuyFrames().size() - 2 - relativeGuyIndex;
    hg::Frame *const guyFrame{ getGuyFrames()[guyIndex] };
    if (!isNullFrame(guyFrame)) {
        hg::GuyOutputInfo const &currentGuy(findCurrentGuy(guyFrame->getView().getGuyInformation(), guyIndex));
        guyDirection = currentGuy.getTimeDirection();
        guyFrameNumber = getFrameNumber(guyFrame);
    }
    else {
        guyFrameNumber = guyFrameNumber + guyDirection;
    }

    size_t minWaveChanges = 0;
    bool first = true;
    for (hg::FrameUpdateSet const &updateSet : updatedList) {
        int waveChanges = 0;
        for (Frame *frame : updateSet) {
            if (guyDirection == TimeDirection::FORWARDS) {
                if (getFrameNumber(frame) < guyFrameNumber) {
                    waveChanges += 1;
                }
            }
            else if (guyDirection == TimeDirection::REVERSE) {
                if (getFrameNumber(frame) > guyFrameNumber) {
                    waveChanges += 1;
                }
            }
        }
        //std::cerr << "updateSize " << updateSize << "\n";

        if (first || waveChanges < minWaveChanges) {
            first = false;
            minWaveChanges = waveChanges;
        }
    }
    //std::cerr << "minWaveChanges " << minWaveChanges << "\n";

    if (minWaveChanges > 0) {
        paradoxPressure += static_cast<int>(std::pow(static_cast<float>(6 * minWaveChanges), 1.2f)) + hg::PARADOX_PRESSURE_ADD_MIN;
        paradoxPressureDecay = 0;
    }
    else if (paradoxPressure > 0) {
        paradoxPressureDecay = std::min(hg::PARADOX_PRESSURE_DECAY_MAX, paradoxPressureDecay + hg::PARADOX_PRESSURE_DECAY_BUILDUP);
        paradoxPressure = std::max(0, paradoxPressure - paradoxPressureDecay);
    }

    prevRunResult = RunResult{ std::move(updatedList), paradoxPressure, static_cast<int>(minWaveChanges) };
    return prevRunResult;
}

TimeEngine::RunResult TimeEngine::getPrevRunResult()
{
    return prevRunResult;
}

Frame const *TimeEngine::getFrame(FrameID const &whichFrame) const noexcept { return impl->worldState.getFrame(whichFrame); }
std::vector<Frame *> const &TimeEngine::getGuyFrames() const noexcept { return impl->worldState.getGuyFrames(); }
std::vector<GuyInput> const &TimeEngine::getPostOverwriteInput() const noexcept { return impl->worldState.getPostOverwriteInput(); }
std::vector<InputList> const &TimeEngine::getReplayData() const noexcept { return impl->worldState.getReplayData(); }
Wall const &TimeEngine::getWall() const noexcept { return impl->wall; }
int TimeEngine::getTimelineLength() const noexcept { return impl->worldState.getTimelineLength(); }


TimeEngine::TimeEngine(TimeEngine const &) = default;
TimeEngine::TimeEngine(TimeEngine &&) noexcept = default;
TimeEngine &TimeEngine::operator=(TimeEngine const&) = default;
TimeEngine &TimeEngine::operator=(TimeEngine &&) noexcept = default;
TimeEngine::~TimeEngine() noexcept = default;


}//namespace hg
