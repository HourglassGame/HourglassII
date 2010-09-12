#ifndef HG_PAUSE_INITIATOR_ID
#define HG_PAUSE_INITIATOR_ID
#include <boost/functional/hash.hpp>

namespace hg {
    namespace pauseinitiatortype {
        enum PauseInitiatorType {
            GUY,
            TRIGGER
        };
    }

    class PauseInitiatorID
    {
    public:
        PauseInitiatorID(pauseinitiatortype::PauseInitiatorType type,
                         unsigned int ID,
                         unsigned int timelineLength);
        pauseinitiatortype::PauseInitiatorType type_;
        unsigned int ID_;
        unsigned int timelineLength_;
    };

    bool operator==(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);
    
    bool operator<(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);
    
    ::std::size_t hash_value(const PauseInitiatorID& toHash);
}
#endif //HG_PAUSE_INITIATOR_ID
