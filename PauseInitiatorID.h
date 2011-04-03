#ifndef HG_PAUSE_INITIATOR_ID
#define HG_PAUSE_INITIATOR_ID
#include <boost/serialization/nvp.hpp>
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
                         size_t ID,
                         size_t timelineLength);
        PauseInitiatorID() :
        type_(pauseinitiatortype::INVALID),
        ID_(0),
        timelineLength_(0)
        {}
        pauseinitiatortype::PauseInitiatorType type_;
        size_t ID_;
        size_t timelineLength_;
        friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int /*version*/)
    {
        ar & BOOST_SERIALIZATION_NVP(type_);
        ar & BOOST_SERIALIZATION_NVP(ID_);
        ar & BOOST_SERIALIZATION_NVP(timelineLength_);
    }
    };

    bool operator==(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);

    bool operator<(const PauseInitiatorID& lhs, const PauseInitiatorID& rhs);

    ::std::size_t hash_value(const PauseInitiatorID& toHash);
}
#endif //HG_PAUSE_INITIATOR_ID
