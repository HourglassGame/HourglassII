#include "GlitzPersister.h"
#include "Frame.h"
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
namespace hg {
GlitzPersister::GlitzPersister(
    Glitz const& forwardsGlitz, Glitz const& reverseGlitz,
    unsigned lifetime, TimeDirection timeDirection) :
        forwardsGlitz_(forwardsGlitz), reverseGlitz_(reverseGlitz),
        framesLeft_(lifetime), timeDirection_(timeDirection)
{
}
ObjectAndTime<GlitzPersister, Frame *> GlitzPersister::runStep(Frame *frame) const
{
    return ObjectAndTime<GlitzPersister, Frame*>(
        GlitzPersister(
            forwardsGlitz_, reverseGlitz_,
            framesLeft_ - 1, timeDirection_),
        framesLeft_ ? nextFrame(frame, timeDirection_) : 0);
}
Glitz const& GlitzPersister::getForwardsGlitz() const{ return forwardsGlitz_; }
Glitz const& GlitzPersister::getReverseGlitz() const { return reverseGlitz_; }
bool GlitzPersister::operator==(GlitzPersister const& o) const
{
    return
        boost::tie(
            forwardsGlitz_, reverseGlitz_,
            framesLeft_,timeDirection_)
        ==
        boost::tie(
            o.forwardsGlitz_, o.reverseGlitz_,
            o.framesLeft_, o.timeDirection_);
}
bool GlitzPersister::operator<(GlitzPersister const& o) const
{
    return
        boost::tie(
            forwardsGlitz_, reverseGlitz_,
            framesLeft_,timeDirection_)
        <
        boost::tie(
            o.forwardsGlitz_, o.reverseGlitz_,
            o.framesLeft_, o.timeDirection_);
}
}
