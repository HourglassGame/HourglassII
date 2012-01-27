#ifndef HG_GLITZ_PERSISTER_H
#define HG_GLITZ_PERSISTER_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include <boost/move/move.hpp>
#include "ObjectAndTime.h"
#include "Glitz.h"
#include "Frame_fwd.h"
#include "ConstPtr_of_fwd.h"

namespace hg {
class GlitzPersister : boost::totally_ordered<GlitzPersister>
{
public:
    GlitzPersister(GlitzPersister const& other) :
        forwardsGlitz_(other.forwardsGlitz_),
        reverseGlitz_(other.reverseGlitz_),
        framesLeft_(other.framesLeft_),
        timeDirection_(other.timeDirection_)
    {}
    GlitzPersister(BOOST_RV_REF(GlitzPersister) other) :
        forwardsGlitz_(boost::move(other.forwardsGlitz_)),
        reverseGlitz_(boost::move(other.reverseGlitz_)),
        framesLeft_(boost::move(other.framesLeft_)),
        timeDirection_(boost::move(other.timeDirection_))
    {}
    GlitzPersister& operator=(GlitzPersister const& other)
    {
        forwardsGlitz_ = other.forwardsGlitz_;
        reverseGlitz_ = other.reverseGlitz_;
        framesLeft_ = other.framesLeft_;
        timeDirection_ = other.timeDirection_;
        return *this;
    }
    GlitzPersister& operator=(BOOST_RV_REF(GlitzPersister) other)
    {
        forwardsGlitz_ = boost::move(other.forwardsGlitz_);
        reverseGlitz_ = boost::move(other.reverseGlitz_);
        framesLeft_ = boost::move(other.framesLeft_);
        timeDirection_ = boost::move(other.timeDirection_);
        return *this;
    }
    
    GlitzPersister(Glitz const& forwardsGlitz, Glitz const& reverseGlitz, unsigned lifetime, TimeDirection timeDirection);
    ObjectAndTime<GlitzPersister, Frame*> runStep(Frame* frame) const;
    Glitz const& getForwardsGlitz() const;
    Glitz const& getReverseGlitz() const;
    
    bool operator==(GlitzPersister const& other) const;
    bool operator<(GlitzPersister const& second) const;
private:
    Glitz forwardsGlitz_;
    Glitz reverseGlitz_;
    unsigned framesLeft_;
    TimeDirection timeDirection_;
    BOOST_COPYABLE_AND_MOVABLE(GlitzPersister)
};

class GlitzPersisterConstPtr : boost::totally_ordered<GlitzPersisterConstPtr>
{
public:
    GlitzPersisterConstPtr(GlitzPersister const& glitzPersister) : glitzPersister_(&glitzPersister) {}
    typedef GlitzPersister base_type;
    GlitzPersister const& get() const   { return *glitzPersister_; }
    
    ObjectAndTime<GlitzPersister, Frame*> runStep(Frame* frame) const { return glitzPersister_->runStep(frame); }
    Glitz const& getForwardsGlitz() const { return glitzPersister_->getForwardsGlitz(); }
    Glitz const& getReverseGlitz() const { return glitzPersister_->getReverseGlitz(); }
    
    bool operator==(GlitzPersisterConstPtr const& other) const { return *glitzPersister_ == *other.glitzPersister_; }
    bool operator<(GlitzPersisterConstPtr const& other) const { return *glitzPersister_ < *other.glitzPersister_; }
private:
    GlitzPersister const* glitzPersister_;
};

template<>
struct ConstPtr_of<GlitzPersister> {
    typedef GlitzPersisterConstPtr type;
};

}
#endif //HG_GLITZ_PERSISTER_H
