#include "GlitzPersister.h"
#include "Frame.h"
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
namespace hg {
GlitzPersister::GlitzPersister(
    Glitz const &forwardsGlitz, Glitz const &reverseGlitz,
    unsigned lifetime, TimeDirection timeDirection) :
        forwardsGlitz(forwardsGlitz), reverseGlitz(reverseGlitz),
        framesLeft(lifetime), timeDirection(timeDirection)
{
}
ObjectAndTime<GlitzPersister, Frame *> GlitzPersister::runStep(Frame *frame) const
{
    return ObjectAndTime<GlitzPersister, Frame*>(
        GlitzPersister(
            forwardsGlitz, reverseGlitz,
            framesLeft - 1, timeDirection),
        framesLeft ? nextFrame(frame, timeDirection) : nullptr);
}
bool GlitzPersister::operator==(GlitzPersister const &o) const
{
    return as_tie() == o.as_tie();
}
bool GlitzPersister::operator<(GlitzPersister const &o) const
{
    return as_tie() < o.as_tie();
}
}
