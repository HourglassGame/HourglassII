#ifndef HG_TRIGGER_DATA_H
#define HG_TRIGGER_DATA_H
#include "SortWeakerThanEquality_fwd.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
#include <vector>
#include <cstddef>
namespace hg
{
class TriggerData : boost::totally_ordered<TriggerData>
{
public:
    TriggerData(std::size_t index, std::vector<int> const& value);

    std::size_t getIndex() const { return index_; }
    std::vector<int> const& getValue() const { return value_; }
    
    bool operator==(const TriggerData& other) const;
    bool operator<(const TriggerData& second) const;
    
private:
    std::size_t index_;
    std::vector<int> value_;

};
class TriggerDataConstPtr : boost::totally_ordered<TriggerDataConstPtr>
{
public:
    TriggerDataConstPtr(TriggerData const& triggerData) : triggerData_(&triggerData) {}
    typedef TriggerData base_type;
    TriggerData const& get() const { return *triggerData_; }
    std::size_t getIndex() const { return triggerData_->getIndex(); }
    std::vector<int> const& getValue()         const { return triggerData_->getValue(); }
    
    bool operator==(const TriggerDataConstPtr& other) const { return *triggerData_ < *other.triggerData_; }
    bool operator<(const TriggerDataConstPtr& other) const { return *triggerData_ < *other.triggerData_;}
    
private:
    TriggerData const* triggerData_;

};
template<>
struct ConstPtr_of<TriggerData> {
    typedef TriggerDataConstPtr type;
};
template<>
struct sort_weaker_than_equality<TriggerData>
{
    static const bool value = true;
};
template<>
struct sort_weaker_than_equality<TriggerDataConstPtr>
{
    static const bool value = true;
};
}
#endif //HG_TRIGGER_DATA_H
