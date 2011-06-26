#ifndef HG_PAUSE_INITIATOR_ID
#define HG_PAUSE_INITIATOR_ID
#include <boost/functional/hash_fwd.hpp>
#include <boost/operators.hpp>
#include <cstddef>
namespace hg {
namespace pauseinitiatortype {
enum PauseInitiatorType {
    INVALID,
    GUY,
    TRIGGER
};
}

class PauseInitiatorID : boost::totally_ordered<PauseInitiatorID>
{
public:
    PauseInitiatorID(
        pauseinitiatortype::PauseInitiatorType type,
        std::size_t ID,
        std::size_t timelineLength);
    PauseInitiatorID() :
            type_(pauseinitiatortype::INVALID),
            ID_(0),
            timelineLength_(0)
    {}
    bool operator==(PauseInitiatorID const& other) const;
    bool operator<(PauseInitiatorID const& other) const;
    pauseinitiatortype::PauseInitiatorType type_;
    std::size_t ID_;
    std::size_t timelineLength_;
};

std::size_t hash_value(PauseInitiatorID const& toHash);
}
#endif //HG_PAUSE_INITIATOR_ID
