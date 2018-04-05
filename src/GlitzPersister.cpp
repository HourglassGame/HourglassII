#include "GlitzPersister.h"
#include "Frame.h"
#include "mt/std/memory"
#include <boost/polymorphic_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "AudioGlitz.h"
namespace hg {
StaticGlitzPersister::StaticGlitzPersister(
    Glitz const &forwardsGlitz, Glitz const &reverseGlitz,
    unsigned lifetime, TimeDirection timeDirection) :
        forwardsGlitz(forwardsGlitz), reverseGlitz(reverseGlitz),
        framesLeft(lifetime), timeDirection(timeDirection)
{
}
ObjectAndTime<GlitzPersister, Frame *> StaticGlitzPersister::runStep(Frame *frame) const
{
    return ObjectAndTime<GlitzPersister, Frame*>(
        GlitzPersister(
            mt::std::make_unique<StaticGlitzPersister>(
                forwardsGlitz, reverseGlitz,
                framesLeft - 1, timeDirection)),
        framesLeft ? nextFrame(frame, timeDirection) : nullptr);
}
bool StaticGlitzPersister::operator==(GlitzPersisterImpl const &o) const
{
    StaticGlitzPersister const &actual_other(*boost::polymorphic_downcast<StaticGlitzPersister const*>(&o));
    return comparison_tuple() == actual_other.comparison_tuple();
}
bool StaticGlitzPersister::operator<(GlitzPersisterImpl const &o) const
{
    StaticGlitzPersister const &actual_other(*boost::polymorphic_downcast<StaticGlitzPersister const*>(&o));
    return comparison_tuple() < actual_other.comparison_tuple();
}

AudioGlitzPersister::AudioGlitzPersister(
        mt::std::string key,
        unsigned duration,
        TimeDirection timeDirection) :
    key(std::move(key)),
    duration(duration),
    currentFrame(0),
    timeDirection(timeDirection)
{}

AudioGlitzPersister::AudioGlitzPersister(
        AudioGlitzPersister_access,
        mt::std::string key,
        unsigned duration,
        unsigned currentFrame,
        TimeDirection timeDirection) :
    key(std::move(key)),
    duration(duration),
    currentFrame(currentFrame),
    timeDirection(timeDirection)
{}

Glitz AudioGlitzPersister::getForwardsGlitz() const  {
    mt::std::string suffix = timeDirection == TimeDirection::FORWARDS ? "" : "_r";
    return Glitz(mt::std::make_unique<AudioGlitz>(
            key+suffix,
            timeDirection == TimeDirection::FORWARDS ? currentFrame : duration-currentFrame));
}
Glitz AudioGlitzPersister::getReverseGlitz() const  {
    mt::std::string suffix = timeDirection == TimeDirection::REVERSE ? "" : "_r";
    return Glitz(mt::std::make_unique<AudioGlitz>(
            key+suffix,
            timeDirection == TimeDirection::REVERSE ? currentFrame : duration-currentFrame));
}

ObjectAndTime<GlitzPersister, Frame *> AudioGlitzPersister::runStep(Frame *frame) const
{
    return ObjectAndTime<GlitzPersister, Frame*>(
        GlitzPersister(
            mt::std::make_unique<AudioGlitzPersister>(
                AudioGlitzPersister_access{},
                key,
                duration,
                currentFrame + 1,
                timeDirection)),
        currentFrame != duration ? nextFrame(frame, timeDirection) : nullptr);
}
bool AudioGlitzPersister::operator==(GlitzPersisterImpl const &o) const
{
    AudioGlitzPersister const &actual_other(*boost::polymorphic_downcast<AudioGlitzPersister const*>(&o));
    return comparison_tuple() == actual_other.comparison_tuple();
}
bool AudioGlitzPersister::operator<(GlitzPersisterImpl const &o) const
{
    AudioGlitzPersister const &actual_other(*boost::polymorphic_downcast<AudioGlitzPersister const*>(&o));
    return comparison_tuple() < actual_other.comparison_tuple();
}


}
