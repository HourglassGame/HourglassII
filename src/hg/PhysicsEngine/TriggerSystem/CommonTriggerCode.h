#ifndef HG_COMMON_TRIGGER_CODE_H
#define HG_COMMON_TRIGGER_CODE_H
#include "hg/mt/std/vector"
#include "hg/mp/std/vector"
#include "hg/TimeEngine/ArrivalDepartures/TriggerData.h"
#include "hg/TimeEngine/ArrivalDepartures/ObjectPtrList.h"
#include "hg/TimeEngine/Frame.h"
#include "hg/TimeEngine/Universe.h"
#include "hg/Util/multi_thread_allocator.h"

#include <utility>

namespace hg {

typedef boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr> const> TriggerDataRange;
            
inline mp::std::vector<mp::std::vector<int>>
    calculateApparentTriggers(
        std::vector<std::pair<int, std::vector<int>>> const &triggerOffsetsAndDefaults,
        TriggerDataRange const &triggerArrivals,
        memory_pool<user_allocator_tbb_alloc> &pool)
{
    //trigger arrivals with defaults for places where none arrived in triggerArrivals
    //index field replaced by position in list.
    mp::std::vector<mp::std::vector<int>> apparentTriggers(pool);
    apparentTriggers.reserve(boost::size(triggerOffsetsAndDefaults));
    typedef std::pair<int, std::vector<int> > TriggerOffsetAndDefault;
    for (TriggerOffsetAndDefault const &offsetAndDefault: triggerOffsetsAndDefaults) {
        apparentTriggers.push_back(
            mp::std::vector<int>(
                    offsetAndDefault.second.begin(),
                    offsetAndDefault.second.end(),
                    pool));
    }
    
    for (TriggerData const &arrival: triggerArrivals) {
        apparentTriggers[arrival.getIndex()].assign(arrival.getValue().begin(), arrival.getValue().end());
    }
    return apparentTriggers;
}
//TODO: move TriggerData from triggers; rather than copying?
inline mt::std::map<Frame *, mt::std::vector<TriggerData>>
calculateActualTriggerDepartures(
    mp::std::vector<TriggerData> const &triggers,
    std::vector<std::pair<int, std::vector<int>>> const &triggerOffsetsAndDefaults,
    Frame *currentFrame)
{
    mt::std::map<Frame *, mt::std::vector<TriggerData>> retv;
    Universe &universe(getUniverse(currentFrame));
    for (TriggerData const& trigger: triggers)
    {
        retv[
            getArbitraryFrame(
                universe, 
                getFrameNumber(currentFrame) + triggerOffsetsAndDefaults[trigger.getIndex()].first)]
          .push_back(trigger);
    }
    //In theory this will be sorted by
    //SortObjectList at the end of
    //PhysicsEngine::executeFrame.
    //However, sorting here allows
    //different TriggerSystem implementations
    //to add trigger data in a slightly different order, but
    //still be considered to have identical behaviour by ComparisonTestTriggerSystem.
    for (auto &[frame, tdVec] : retv) {
        boost::sort(tdVec);
    }
    return retv;
}

}
#endif //HG_COMMON_TRIGGER_CODE_H

