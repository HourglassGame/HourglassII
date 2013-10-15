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
    GlitzPersister(GlitzPersister const &o) :
        forwardsGlitz_(o.forwardsGlitz_),
        reverseGlitz_(o.reverseGlitz_),
        framesLeft_(o.framesLeft_),
        timeDirection_(o.timeDirection_)
    {}
    GlitzPersister(BOOST_RV_REF(GlitzPersister) o) :
        forwardsGlitz_(boost::move(o.forwardsGlitz_)),
        reverseGlitz_(boost::move(o.reverseGlitz_)),
        framesLeft_(boost::move(o.framesLeft_)),
        timeDirection_(boost::move(o.timeDirection_))
    {}
    GlitzPersister &operator=(GlitzPersister const &o)
    {
        forwardsGlitz_ = o.forwardsGlitz_;
        reverseGlitz_ = o.reverseGlitz_;
        framesLeft_ = o.framesLeft_;
        timeDirection_ = o.timeDirection_;
        return *this;
    }
    GlitzPersister &operator=(BOOST_RV_REF(GlitzPersister) o)
    {
        forwardsGlitz_ = boost::move(o.forwardsGlitz_);
        reverseGlitz_ = boost::move(o.reverseGlitz_);
        framesLeft_ = boost::move(o.framesLeft_);
        timeDirection_ = boost::move(o.timeDirection_);
        return *this;
    }
    
    GlitzPersister(
        Glitz const &forwardsGlitz, Glitz const &reverseGlitz,
        unsigned lifetime, TimeDirection timeDirection);
    ObjectAndTime<GlitzPersister, Frame*> runStep(Frame *frame) const;
    Glitz const &getForwardsGlitz() const;
    Glitz const &getReverseGlitz() const;
    
    bool operator==(GlitzPersister const &o) const;
    bool operator<(GlitzPersister const &second) const;
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
    GlitzPersisterConstPtr(GlitzPersister const &glitzPersister) : glitzPersister_(&glitzPersister) {}
    typedef GlitzPersister base_type;
    GlitzPersister const &get() const   { return *glitzPersister_; }
    
    ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const { return glitzPersister_->runStep(frame); }
    Glitz const &getForwardsGlitz() const { return glitzPersister_->getForwardsGlitz(); }
    Glitz const &getReverseGlitz() const { return glitzPersister_->getReverseGlitz(); }
    
    bool operator==(GlitzPersisterConstPtr const &o) const { return *glitzPersister_ == *o.glitzPersister_; }
    bool operator<(GlitzPersisterConstPtr const &o) const { return *glitzPersister_ < *o.glitzPersister_; }
private:
    GlitzPersister const *glitzPersister_;
};

template<>
struct ConstPtr_of<GlitzPersister> {
    typedef GlitzPersisterConstPtr type;
};

}
#endif //HG_GLITZ_PERSISTER_H
