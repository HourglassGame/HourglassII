#ifndef HG_TRIGGER_OFFSETS_AND_DEFAULTS_H
#define HG_TRIGGER_OFFSETS_AND_DEFAULTS_H
#include <vector>
#include <utility>
namespace hg {
struct TriggerOffsetsAndDefaults {
    explicit TriggerOffsetsAndDefaults(
        std::vector<std::pair<int,std::vector<int> > > const& ntoad)
            : toad(ntoad) {}
    std::vector<std::pair<int,std::vector<int> > > toad;
};
}
#endif //HG_TRIGGER_OFFSETS_AND_DEFAULTS_H
