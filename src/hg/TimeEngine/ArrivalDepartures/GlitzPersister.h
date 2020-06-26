#ifndef HG_GLITZ_PERSISTER_H
#define HG_GLITZ_PERSISTER_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "hg/mt/std/string"
#include "hg/mt/std/memory"
#include <tuple>
#include "hg/TimeEngine/ObjectAndTime.h"
#include "hg/TimeEngine/Glitz/Glitz.h"
#include "hg/TimeEngine/Frame_fwd.h"
#include "ConstPtr_of_fwd.h"

namespace hg {
class GlitzPersister;
struct GlitzPersisterImpl : private boost::totally_ordered<GlitzPersisterImpl>
{
	virtual ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const = 0;
	virtual Glitz getForwardsGlitz() const = 0;
	virtual Glitz getReverseGlitz()  const = 0;
	
	virtual std::size_t clone_size() const = 0;
	virtual GlitzPersisterImpl *perform_clone(void *memory) const = 0;
	virtual ~GlitzPersisterImpl() noexcept {}
	
	virtual int order_ranking() const = 0;
	virtual bool operator==(GlitzPersisterImpl const &o) const = 0;
	virtual bool operator<(GlitzPersisterImpl const &o) const = 0;
};

class GlitzPersister final : boost::totally_ordered<GlitzPersister>
{
public:
	explicit GlitzPersister(mt::std::unique_ptr<GlitzPersisterImpl> impl) :
		impl(impl.release())
	{
		assert(this->impl);
	}
	
	ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const {
		return impl->runStep(frame);
	}
	
	Glitz getForwardsGlitz() const { return impl->getForwardsGlitz(); }
	Glitz getReverseGlitz()  const { return impl->getReverseGlitz(); }
	
	bool operator==(GlitzPersister const &o) const {
		return comparison_tuple() == o.comparison_tuple();
	}
	bool operator<(GlitzPersister const &o) const {
		return comparison_tuple() < o.comparison_tuple();
	}
private:
	clone_ptr<GlitzPersisterImpl, memory_source_clone<GlitzPersisterImpl, multi_thread_memory_source>> impl;
	typedef
	  std::tuple<
		decltype(impl->order_ranking()),
		GlitzPersisterImpl const &>
	  comparison_tuple_type;
	comparison_tuple_type comparison_tuple() const {
		return comparison_tuple_type(impl->order_ranking(), *impl);
	}
};

class GlitzPersisterConstPtr final : boost::totally_ordered<GlitzPersisterConstPtr>
{
public:
	explicit GlitzPersisterConstPtr(GlitzPersister const &glitzPersister) : glitzPersister(&glitzPersister) {}
	typedef GlitzPersister base_type;
	GlitzPersister const &get() const   { return *glitzPersister; }
	
	ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const { return glitzPersister->runStep(frame); }
	Glitz getForwardsGlitz() const { return glitzPersister->getForwardsGlitz(); }
	Glitz getReverseGlitz() const { return glitzPersister->getReverseGlitz(); }
	
	bool operator==(GlitzPersisterConstPtr const &o) const { return *glitzPersister == *o.glitzPersister; }
	bool operator< (GlitzPersisterConstPtr const &o) const { return *glitzPersister <  *o.glitzPersister; }
private:
	GlitzPersister const *glitzPersister;
};

template<>
struct ConstPtr_of<GlitzPersister> {
	typedef GlitzPersisterConstPtr type;
};

class StaticGlitzPersister final : public GlitzPersisterImpl
{
public:
	explicit StaticGlitzPersister(
		Glitz const &forwardsGlitz, Glitz const &reverseGlitz,
		unsigned lifetime, TimeDirection timeDirection);
	ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const override;
	Glitz getForwardsGlitz() const override { return forwardsGlitz; }
	Glitz getReverseGlitz()  const override { return reverseGlitz; }
	
	virtual std::size_t clone_size() const override {
		return sizeof *this;
	}
	virtual StaticGlitzPersister *perform_clone(void *memory) const override {
		return new (memory) StaticGlitzPersister(*this);
	}
	virtual ~StaticGlitzPersister() noexcept override {}
	virtual int order_ranking() const override {
		return 0;
	}
	bool operator==(GlitzPersisterImpl const &o) const override;
	bool operator<(GlitzPersisterImpl const &o) const override;
private:
	Glitz forwardsGlitz;
	Glitz reverseGlitz;
	int framesLeft; // peristers with framesLeft = -1 are permantent
	TimeDirection timeDirection;
	auto comparison_tuple() const -> decltype(auto)
	{
		return std::tie(
			forwardsGlitz, reverseGlitz,
			framesLeft,timeDirection);
	}
};

class AudioGlitzPersister final : public GlitzPersisterImpl
{
	struct AudioGlitzPersister_access final{
		friend class AudioGlitzPersister;
	private:
		AudioGlitzPersister_access(){}
	};
public:
	explicit AudioGlitzPersister(
		mt::std::string key,
		unsigned duration,
		TimeDirection timeDirection);

	ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const override;
	Glitz getForwardsGlitz() const override;
	Glitz getReverseGlitz()  const override;
	
	virtual std::size_t clone_size() const override {
		return sizeof *this;
	}
	virtual AudioGlitzPersister *perform_clone(void *memory) const override {
		return new (memory) AudioGlitzPersister(*this);
	}
	virtual ~AudioGlitzPersister() noexcept override {}
	virtual int order_ranking() const override {
		return 1;
	}
	bool operator==(GlitzPersisterImpl const &o) const override;
	bool operator<(GlitzPersisterImpl const &o) const override;
	
	//Mostly private. Only accessible via AudioGlitzPersister_access key, which can only be created by
	//AudioGlitzPersister.
	AudioGlitzPersister(
		AudioGlitzPersister_access,
		mt::std::string key,
		unsigned duration,
		unsigned currentFrame,
		TimeDirection timeDirection);
private:
	mt::std::string key;
	unsigned duration;
	unsigned currentFrame;
	TimeDirection timeDirection;

	auto comparison_tuple() const -> decltype(auto)
	{
		return std::tie(
			key,
			duration,
			currentFrame,
			timeDirection);
	}
};
}
#endif //HG_GLITZ_PERSISTER_H
