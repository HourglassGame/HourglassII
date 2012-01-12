#include "GlitzPersister.h"
#include "Frame.h"
namespace hg {
GlitzPersister::GlitzPersister(Glitz const& glitz, unsigned lifetime, TimeDirection timeDirection) :
    glitz_(glitz), framesLeft_(lifetime), timeDirection_(timeDirection)
{
}
ObjectAndTime<GlitzPersister, Frame*> GlitzPersister::runStep(Frame* frame) const
{
    return ObjectAndTime<GlitzPersister, Frame*>(
        GlitzPersister(glitz_, framesLeft_ - 1, timeDirection_),
        framesLeft_ ? nextFrame(frame, timeDirection_) : 0);
}
Glitz const& GlitzPersister::getGlitz() const
{
    return glitz_;
}

bool GlitzPersister::operator==(GlitzPersister const& other) const
{
    return glitz_ == other.glitz_
        && framesLeft_ == other.framesLeft_
        && timeDirection_ == other.timeDirection_;
}
bool GlitzPersister::operator<(GlitzPersister const& second) const
{
    if (glitz_ == second.glitz_) {
        if (framesLeft_ == second.framesLeft_) {
            return timeDirection_ < second.timeDirection_;
        }
        return framesLeft_ < second.framesLeft_;
    }
    return glitz_ < second.glitz_;
}
}
