#ifndef HG_PAUSE_INITIATOR_ID
#define HG_PAUSE_INITIATOR_ID
#include <cstddef>

namespace hg {
namespace pauseinitiatortype {
enum PauseInitiatorType {
    INVALID,
    GUY,
    TRIGGER
};
}

class PauseInitiatorID
{
public:
    PauseInitiatorID(pauseinitiatortype::PauseInitiatorType type,
                     std::size_t ID,
                     std::size_t timelineLength);
    PauseInitiatorID() :
            type_(pauseinitiatortype::INVALID),
            ID_(0),
            timelineLength_(0)
    {}
    pauseinitiatortype::PauseInitiatorType type_;
    std::size_t ID_;
    std::size_t timelineLength_;
};

bool operator==(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);

bool operator<(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);

std::size_t hash_value(const PauseInitiatorID& toHash);
}
#endif //HG_PAUSE_INITIATOR_ID
