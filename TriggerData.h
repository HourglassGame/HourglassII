#ifndef HG_TRIGGER_DATA_H
#define HG_TRIGGER_DATA_H
#include "SortWeakerThanEquality_fwd.h"
#include <cstddef>
namespace hg
{
class TriggerData
{
public:
    TriggerData(std::size_t index, int value);
    TriggerData(const TriggerData& other);

    TriggerData& operator=(const TriggerData& other);

    std::size_t getIndex() const {
        return index_;
    }
    int getValue() const {
        return value_;
    }
    
    bool operator==(const TriggerData& other) const;
    bool operator!=(const TriggerData& other) const;

    bool operator<(const TriggerData& second) const;

private:
    std::size_t index_;
    int value_;

};
template<>
struct sort_weaker_than_equality<TriggerData>
{
    static const bool value = true;
};
}
#endif //HG_TRIGGER_DATA_H
