#ifndef HG_COMMON_TRIGGER_CODE_H
#define HG_COMMON_TRIGGER_CODE_H
#include "mt/std/vector"
#include "TriggerData.h"
#include "ObjectPtrList.h"
#include "Frame.h"
#include "Universe.h"
#include "multi_thread_allocator.h"

#include <utility>

namespace hg {

typedef boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr> const> TriggerDataRange;
            
inline mt::std::vector<mt::std::vector<int>>
    calculateApparentTriggers(
        std::vector<std::pair<int, std::vector<int> > > const &triggerOffsetsAndDefaults,
        TriggerDataRange const &triggerArrivals)
{
    //trigger arrivals with defaults for places where none arrived in triggerArrivals
    //index field replaced by position in list.
    mt::std::vector<mt::std::vector<int>> apparentTriggers;
    apparentTriggers.reserve(boost::distance(triggerOffsetsAndDefaults));
    typedef std::pair<int, std::vector<int> > TriggerOffsetAndDefault;
    for (TriggerOffsetAndDefault const &offsetAndDefault: triggerOffsetsAndDefaults) {
        apparentTriggers.push_back(
            mt::std::vector<int>(
                    offsetAndDefault.second.begin(),
                    offsetAndDefault.second.end()));
    }
    
    for (TriggerData const &arrival: triggerArrivals) {
        apparentTriggers[arrival.getIndex()] = arrival.getValue();
    }
    return apparentTriggers;
}

inline mt::std::map<Frame *, mt::std::vector<TriggerData>>
calculateActualTriggerDepartures(
    mt::std::vector<TriggerData> const &triggers,
    std::vector<std::pair<int, std::vector<int> > > const &triggerOffsetsAndDefaults,
    Frame *currentFrame)
{
    mt::std::map<Frame *, mt::std::vector<TriggerData>> retv;
    Universe &universe(getUniverse(currentFrame));
    for (TriggerData const& trigger: triggers)
    {
        retv[
            getArbitraryFrame(
                universe, 
                getFrameNumber(currentFrame) + triggerOffsetsAndDefaults[trigger.getIndex()].first)].push_back(
                    trigger);
    }
    return retv;
}

}
#endif //HG_COMMON_TRIGGER_CODE_H

