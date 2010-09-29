#ifndef HG_PAUSE_INITIATOR_ID
#define HG_PAUSE_INITIATOR_ID
#include <cstring>
namespace boost {
namespace serialization {
class access;
}
}
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
                         unsigned int ID,
                         unsigned int timelineLength);
        PauseInitiatorID() :
        type_(pauseinitiatortype::INVALID),
        ID_(0),
        timelineLength_(0)
        {}
        pauseinitiatortype::PauseInitiatorType type_;
        unsigned int ID_;
        unsigned int timelineLength_;
        friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & type_;
        ar & ID_;
        ar & timelineLength_;
    }
    };

    bool operator==(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);
    
    bool operator<(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);
    
    ::std::size_t hash_value(const PauseInitiatorID& toHash);
}
#endif //HG_PAUSE_INITIATOR_ID
