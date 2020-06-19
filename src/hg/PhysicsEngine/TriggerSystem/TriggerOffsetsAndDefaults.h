#ifndef HG_TRIGGER_OFFSETS_AND_DEFAULTS_H
#define HG_TRIGGER_OFFSETS_AND_DEFAULTS_H
#include <vector>
#include <utility>
namespace hg {
struct TriggerOffsetsAndDefaults final {
	std::vector<std::pair<int,std::vector<int>>> value;
};
}
#endif //HG_TRIGGER_OFFSETS_AND_DEFAULTS_H
