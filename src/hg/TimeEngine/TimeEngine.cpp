#include "TimeEngine.h"

#include "Level.h"
#include "Frame.h"
#include "hg/GlobalConst.h"

#include <boost/swap.hpp>
#include <boost/range/algorithm/find_if.hpp>
//#include <iostream>

namespace hg {
struct TimeEngineImpl final {
	TimeEngineImpl *clone() const {
		return new TimeEngineImpl(*this);
	}
	unsigned int speedOfTime;
	unsigned int speedOfTimeFuture;
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
		level.speedOfTimeFuture,
		WorldState(
			level.timelineLength,
			level.speedOfTime,
			level.speedOfTimeFuture,
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

hg::GuyOutputInfo const &findCurrentGuy(mt::std::vector<GuyOutputInfo> const &guyRange, std::size_t const index)
{
	return *boost::find_if(guyRange, [index](auto const& guyInfo) {return guyInfo.getIndex() == index;});
}

TimeEngine::RunResult TimeEngine::runToNextPlayerFrame(InputList const &newInputData, size_t const relativeGuyIndex, OperationInterrupter &interrupter)
{
	impl->worldState.addNewInputData(newInputData);
	FrameListList updatedList;
	updatedList.reserve(impl->speedOfTime);
	
	// Only generate paradox pressure for waves behind the current guy
	std::size_t guyIndex = getGuyFrames().size() - 2 - relativeGuyIndex;
	if (!getGuyFrames()[guyIndex].empty()) {
		bool directionSet = false;
		for (hg::Frame *frame : getGuyFrames()[guyIndex])
		{
			hg::GuyOutputInfo const &currentGuy(findCurrentGuy(frame->getView().getGuyInformation(), guyIndex));
			if (!directionSet) {
				guyDirection = currentGuy.getTimeDirection();
			}
			else if (guyDirection != currentGuy.getTimeDirection()) {
				guyDirection = TimeDirection::INVALID;
				// If two guys exist going in opposite directions, sum paradoxes across the whole timeline.
				break;
			}

			// Paradox wave sum is from the guy furthest in the personal future, when all guys progate in the same direction.
			if (guyDirection == TimeDirection::FORWARDS) {
				if (directionSet) {
					guyFrameNumber = std::max(guyFrameNumber, getFrameNumber(frame));
				}
				else {
					guyFrameNumber = getFrameNumber(frame);
				}
			}
			else if (guyDirection == TimeDirection::REVERSE) {
				if (directionSet) {
					guyFrameNumber = std::min(guyFrameNumber, getFrameNumber(frame));
				}
				else {
					guyFrameNumber = getFrameNumber(frame);
				}
			}

			directionSet = true;
		}
	}
	else {
		guyFrameNumber = guyFrameNumber + guyDirection;
	}
	
	// Find the present Guy
	int arrivalGuyFrameNumber = 0;
	TimeDirection arrivalGuyDirection = TimeDirection::INVALID;
	
	guyIndex = getGuyArrivalFrames().size() - 2 - relativeGuyIndex; // Current Guy
	if (guyDirection != TimeDirection::INVALID) {
		if (!getGuyArrivalFrames()[guyIndex].empty()) {
			arrivalGuyDirection = guyDirection;
			for (hg::Frame *frame : getGuyArrivalFrames()[guyIndex])
			{
				if (guyDirection == TimeDirection::FORWARDS) {
					arrivalGuyFrameNumber = std::max(arrivalGuyFrameNumber, getFrameNumber(frame));
				}
				else {
					arrivalGuyFrameNumber = std::min(arrivalGuyFrameNumber, getFrameNumber(frame));
				}
			}
		}
	}
	
	// Update world with guy frame and direction.
	for (unsigned int i(0); i < impl->speedOfTime && !interrupter.interrupted(); ++i) {
		updatedList.push_back(impl->worldState.executeWorld(interrupter, i, arrivalGuyFrameNumber, arrivalGuyDirection));
	}

	unsigned minWaveChanges = 0;
	unsigned maxFrameParadoxPressure = 0;
	bool first = true;
	for (hg::FrameUpdateSet const &updateSet : updatedList) {
		unsigned waveChanges = 0;
		for (Frame *frame : updateSet) {
			if (guyDirection == TimeDirection::FORWARDS) {
				if (getFrameNumber(frame) < guyFrameNumber) {
					waveChanges += getFrameParadoxPressure(frame);
					if (getFrameParadoxPressure(frame) > maxFrameParadoxPressure) {
						maxFrameParadoxPressure = getFrameParadoxPressure(frame);
					}
				}
			}
			else if (guyDirection == TimeDirection::REVERSE) {
				if (getFrameNumber(frame) > guyFrameNumber) {
					waveChanges += getFrameParadoxPressure(frame);
					if (getFrameParadoxPressure(frame) > maxFrameParadoxPressure) {
						maxFrameParadoxPressure = getFrameParadoxPressure(frame);
					}
				}
			}
			else if (guyDirection == TimeDirection::INVALID) {
				waveChanges += getFrameParadoxPressure(frame);
				if (getFrameParadoxPressure(frame) > maxFrameParadoxPressure) {
					maxFrameParadoxPressure = getFrameParadoxPressure(frame);
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
		paradoxPressure += static_cast<int>(std::pow(static_cast<float>(3 * minWaveChanges), 1.2f)) 
			+ hg::PARADOX_PRESSURE_ADD_MIN * maxFrameParadoxPressure / hg::PARADOX_PRESSURE_PER_FRAME;
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
std::vector<ConcurrentTimeSet> const &TimeEngine::getGuyFrames() const noexcept { return impl->worldState.getGuyFrames(); }
std::vector<std::vector<int> > const &TimeEngine::getFrameGuys() const noexcept { return impl->worldState.getFrameGuys(); }
std::vector<ConcurrentTimeSet> const &TimeEngine::getGuyArrivalFrames() const noexcept { return impl->worldState.getGuyArrivalFrames(); }
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
