#include "TriggerSystem.h"
#include "Frame.h"
#include "Universe.h"
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/call_traits.hpp>
#include <boost/foreach.hpp>
#include <boost/range.hpp>
#define foreach BOOST_FOREACH
namespace hg {
namespace {
//Needs tests, I may have screwed it up.
template<typename Iterator, typename Value>
Iterator binary_find(
    typename boost::call_traits<Iterator>::param_type begin,
    typename boost::call_traits<Iterator>::param_type end,
    typename boost::call_traits<Value>::param_type val)
{
    if (begin == end) {
        return end;
    }
    Iterator mid(begin + (end - begin)/2);
    if (val < *mid) {
        Iterator found(binary_find(begin, mid, val));
        return found == mid ? end : found;
    }
    if (val == *mid) {
        return mid;
    }
    return binary_find(mid, end, val);
}

template<typename RandomAccessRange, typename Value>
typename boost::range_iterator<const RandomAccessRange>::type
binary_find(const RandomAccessRange& rng, typename boost::call_traits<Value>::param_type val)
{
    return binary_find(boost::begin(rng), boost::end(rng), val);
}

template<typename IndexableObject>
struct IndexOf : std::unary_function<const IndexableObject&, std::size_t>
{
    std::size_t operator()(const IndexableObject& toTakeIndexOf) const {
        return toTakeIndexOf.getIndex();
}
};

struct Point {
    Point(int x, int y) :
    x_(x), y_(y) {}
    int getX() const { return x_; }
    int getY() const { return y_; }
    private:
    int x_;
    int y_;
};

template<typename Object,typename PlatformRange>
Point snapAttachment(const Object& toSnap, const PlatformRange& platforms, const Attachment& attachment)
{
    //If platform exists then snap to relative position, otherwise do nothing.
    //Could be binary find, but overhead from such a function would probably make it slower than find
    //(for the number of platforms normally in levels)
    //Also, std/boost::binary_find doesn't exist, and I'm not sure that my version is bug-free/maximally efficient!
    typename boost::range_iterator<const PlatformRange>::type platformIt(
        boost::find(
            platforms | boost::adaptors::transformed(IndexOf<Platform>()),
            attachment.platformIndex).base());
            
    if (platformIt == boost::end(platforms)) {
        return Point(toSnap.getX(), toSnap.getY());
    }
    else {
        return platformIt->getTimeDirection() * toSnap.getTimeDirection() == FORWARDS ?
            Point(
                platformIt->getX() + attachment.xOffset,
                platformIt->getY() + attachment.yOffset): 
            Point(
                platformIt->getX() - platformIt->getXspeed() + attachment.xOffset, 
                platformIt->getY() - platformIt->getYspeed() + attachment.yOffset);
    }
}


template<typename Object>
Object sameButDifferentPointAndVelocity(
    const Point& newPosition,
    const Object& oldArrival)
{
    return Object(
        newPosition.getX(),
        newPosition.getY(),
        newPosition.getX() - oldArrival.getX(),
        newPosition.getY() - oldArrival.getY(),
        oldArrival);
}

//Actually only does calculation based on position,
//the width/height are just added here for stupid reasons
//(because I found it hard to implement cleanly the other way)
template<typename AttachedRange, typename PlatformRange>
std::vector<typename boost::range_value<AttachedRange>::type> snapAttachments(
    const AttachedRange& toSnap,
    const PlatformRange& platforms,
    const std::vector<Attachment>& attachments)
{
    std::vector<typename boost::range_value<AttachedRange>::type> retv;
    retv.reserve(boost::distance(toSnap));
    foreach(const typename boost::range_value<AttachedRange>::type& object, toSnap) {
        retv.push_back(
            sameButDifferentPointAndVelocity(
                snapAttachment(object, platforms, attachments[object.getIndex()]),
                object));
    }
    return retv;
}

template<typename RectA, typename RectB>
bool intersectingExclusive(
    const RectA& recta,
    const RectB& rectb)
{
    int xa(recta.getX());
    int ya(recta.getY());
    int wa(recta.getWidth());
    int ha(recta.getHeight());
    int xb(rectb.getX());
    int yb(rectb.getY());
    int wb(rectb.getWidth());
    int hb(rectb.getHeight());
    return
    (
        (xa < xb && xa + wa > xb)
        ||
        (xb < xa && xb + wb > xa)
        ||
        (xa == xb)
    )
    &&
    (
        (ya < yb && ya + ha > yb)
        ||
        (yb < ya && yb + hb > ya)
        ||
        (ya == yb)
    )
    ;
}

std::vector<char> calculateButtonState(
    unsigned buttonCount,
    const ObjectPtrList& arrivals,
    const std::vector<Attachment>& attachments)
{
    std::vector<Button> snappedButtons(
        snapAttachments(
            arrivals.getButtonListRef(),
            arrivals.getPlatformListRef(),
            attachments));
    std::vector<char> buttonState(buttonCount, false);
    foreach (const Button& button, snappedButtons) {
        foreach(const Box& box, arrivals.getBoxListRef()) {
            if (intersectingExclusive(button, box)) {
                goto intersecting;
            }
        }
        foreach(const Guy& guy, arrivals.getGuyListRef()) {
            if (intersectingExclusive(button, guy)) {
                goto intersecting;
            }
        }
        continue;
        intersecting:
        buttonState[button.getIndex()] = true;
    }
    return buttonState;
}

std::vector<const PlatformDestination*> getPlatformDestinations(
    const std::vector<char>& buttonState,
    const std::vector<PlatformDestinationPair>& destinations)
{
    assert(boost::distance(buttonState) == boost::distance(destinations));
    
    std::vector<const PlatformDestination*> platformDestinations;
    platformDestinations.reserve(buttonState.size());

    for (std::size_t i(0), size(buttonState.size()); i != size; ++i)
    {
        platformDestinations.push_back(
            buttonState[i]?
                &destinations[i].onDestination:
                &destinations[i].offDestination);
    }
    return platformDestinations;
}

template<typename PlatformRange, typename PlatformDestinationRange>
std::vector<Platform> getPlatformDepartures(
    const PlatformRange& platforms,
    const PlatformDestinationRange& destinations)
{
    std::vector<Platform> nextPlatform;
    assert(boost::distance(platforms) <= boost::distance(destinations));
    foreach (const Platform& platform,  platforms)
    {
        const PlatformDestination& destination(
            boost::begin(destinations)[platform.getIndex()]);
        assert(platform.getPauseLevel() == 0);
        
        PositionAndVelocity horizontal(
            destination.
                getXDestination().
                    solvePDEquation(
                        PositionAndVelocity(platform.getX(), platform.getXspeed())));

        PositionAndVelocity vertical(
            destination.
                getYDestination().
                    solvePDEquation(
                        PositionAndVelocity(platform.getY(), platform.getYspeed())));
        
        nextPlatform.push_back(
            Platform(
                horizontal.position, vertical.position,
                horizontal.velocity, vertical.velocity,
                platform.getWidth(), platform.getHeight(),
                platform.getIndex(),
                platform.getTimeDirection(),
                platform.getPauseLevel()));
    }
    return nextPlatform;
}
}//namespace

TriggerSystem::TriggerSystem(
    unsigned int buttonCount,
    const std::vector<PlatformDestinationPair>& destinations,
    const AttachmentMap& attachmentMap) :
        buttonCount_(buttonCount),
        destinations_(destinations),
        attachmentMap_(attachmentMap)
{
    assert(buttonCount_ == destinations_.size());
}

ObjectList TriggerSystem::calculateStaticDepartures(
    const ObjectPtrList& arrivals,
    const std::vector<InputList>& /*playerInput*/,
    const Frame* time) const
{
    (void)time;
    assert(isNullFrame(getInitiatorFrame(getUniverse(time))) && "There really is no way for the trigger system to work with pause time (I think?)");
    std::vector<char> buttonState(calculateButtonState(buttonCount_, arrivals, attachmentMap_.getButtonAttachmentRef()));
    ObjectList retv;
    //TODO - add versions of add which take ranges.
    foreach (
        const Platform& platform,
        getPlatformDepartures(
            arrivals.getPlatformListRef(),
            getPlatformDestinations(buttonState, destinations_) | boost::adaptors::indirected))
    {
        retv.add(platform);
    }
    foreach (
        const Button& button,
        snapAttachments(
            arrivals.getButtonListRef(),
            retv.getPlatformListRef(),
            attachmentMap_.getButtonAttachmentRef())) {
        retv.add(Button(
            button.getX(), button.getY(),
            button.getXspeed(), button.getYspeed(),
            button.getWidth(), button.getHeight(),
            button.getIndex(),
            buttonState[button.getIndex()],
            button.getTimeDirection(),
            button.getPauseLevel()));
    }
    foreach (
        const Portal& portal,
        snapAttachments(
            arrivals.getPortalListRef(),
            retv.getPlatformListRef(),
            attachmentMap_.getPortalAttachmentRef())) {
        retv.add(portal);
    }
    return retv;
}
}//namespace hg
