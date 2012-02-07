#include "GlitzPersister.h"
#include "Frame.h"
namespace hg {
GlitzPersister::GlitzPersister(
    Glitz const& forwardsGlitz, Glitz const& reverseGlitz,
    unsigned lifetime, TimeDirection timeDirection) :
        forwardsGlitz_(forwardsGlitz), reverseGlitz_(reverseGlitz),
        framesLeft_(lifetime), timeDirection_(timeDirection)
{
}
ObjectAndTime<GlitzPersister, Frame*> GlitzPersister::runStep(Frame* frame) const
{
    return ObjectAndTime<GlitzPersister, Frame*>(
        GlitzPersister(forwardsGlitz_, reverseGlitz_, framesLeft_ - 1, timeDirection_),
        framesLeft_ ? nextFrame(frame, timeDirection_) : 0);
}
Glitz const& GlitzPersister::getForwardsGlitz() const
{
    return forwardsGlitz_;
}
Glitz const& GlitzPersister::getReverseGlitz() const
{
    return reverseGlitz_;
}
bool GlitzPersister::operator==(GlitzPersister const& o) const
{
    return forwardsGlitz_ == o.forwardsGlitz_
        && reverseGlitz_ == o.reverseGlitz_
        && framesLeft_ == o.framesLeft_
        && timeDirection_ == o.timeDirection_;
}
bool GlitzPersister::operator<(GlitzPersister const& second) const
{
    if (forwardsGlitz_ == second.forwardsGlitz_) {
        if (reverseGlitz_ == second.reverseGlitz_) {
            if (framesLeft_ == second.framesLeft_) {
                return timeDirection_ < second.timeDirection_;
            }
            return framesLeft_ < second.framesLeft_;
        }
        return reverseGlitz_ < second.reverseGlitz_;
    }
    return forwardsGlitz_ < second.forwardsGlitz_;
}
}
