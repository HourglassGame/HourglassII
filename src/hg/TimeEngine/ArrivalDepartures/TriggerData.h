#ifndef HG_TRIGGER_DATA_H
#define HG_TRIGGER_DATA_H
#include "SortWeakerThanEquality_fwd.h"
#include "ConstPtr_of_fwd.h"
#include "hg/mt/std/vector"
#include <cstddef>
#include <compare>
namespace hg
{
class TriggerData final
{
public:
	TriggerData(std::size_t index, mt::std::vector<int> const &value);

	std::size_t getIndex() const { return index; }
	mt::std::vector<int> const &getValue() const { return value; }
	
	bool operator==(TriggerData const& o) const = default;
	std::weak_ordering operator<=>(TriggerData const &second) const;
	
private:
	std::size_t index;
	mt::std::vector<int> value;
};
class TriggerDataConstPtr final
{
public:
	typedef TriggerData base_type;
	
	TriggerDataConstPtr(TriggerData const &triggerData) : p(&triggerData) {}
	TriggerData const &get() const { return *p; }
	std::size_t getIndex() const { return p->getIndex(); }
	mt::std::vector<int> const &getValue() const { return p->getValue(); }
	
	bool operator==(TriggerDataConstPtr const &o) const { return *p == *o.p;}
	std::weak_ordering operator<=>(TriggerDataConstPtr const &o) const { return *p <=> *o.p;}
	
private:
	TriggerData const *p;
};
template<>
struct ConstPtr_of<TriggerData> {
	typedef TriggerDataConstPtr type;
};
template<>
struct sort_weaker_than_equality<TriggerData> final
{
	static bool const value = true;
};
template<>
struct sort_weaker_than_equality<TriggerDataConstPtr> final
{
	static bool const value = true;
};
}
#endif //HG_TRIGGER_DATA_H
