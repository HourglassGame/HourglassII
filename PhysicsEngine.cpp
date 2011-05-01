#include "PhysicsEngine.h"

#include "ObjectList.h"
#include "TimeDirection.h"
#include "ConcurrentTimeMap.h"
#include "ConcurrentTimeSet.h"

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/foreach.hpp>

#include <map>
#include <vector>

#include <cassert>
#include <cmath>

#define foreach BOOST_FOREACH

namespace hg {
static bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
static bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
static bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

PhysicsEngine::PhysicsEngine(
    const boost::multi_array<bool, 2>& nwallmap,
    int newWallSize,
    int newGravity,
    const AttachmentMap& nAttachmentMap,
    const TriggerSystem& nTriggerSystem) :
        wallmap(nwallmap),
        gravity(newGravity),
        wallSize(newWallSize),
        attachmentMap(nAttachmentMap),
        triggerSystem(nTriggerSystem)
{
}
namespace {
    struct SortObjectList
    {
        void operator()(std::pair<Frame* const, ObjectList>& toSortObjectListOf) const
        {
            toSortObjectListOf.second.sort();
        }
    };
}

PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
    const ObjectPtrList& arrivals,
    Frame* time,
    const std::vector<InputList>& playerInput) const
{
    std::vector<ObjectAndTime<Box> > nextBox;
    std::vector<ObjectAndTime<Guy> > nextGuy;

    std::vector<Platform> nextPlatform;
    nextPlatform.reserve(triggerSystem.getPlatformCount());

    std::vector<Button> nextButton;
    nextButton.reserve(triggerSystem.getButtonCount());

    std::vector<Portal> nextPortal;
    nextPortal.reserve(arrivals.getPortalListRef().size());

    std::vector<PlatformDestination> platformDesinations;

    std::vector<char> nextButtonState(triggerSystem.getButtonCount(), false);

    std::vector<PauseInitiatorID> pauseTimes;

    NewDeparturesT newDepartures;

    if (time->parentFrame() == 0)
    {
        // button state update
        buttonChecks(arrivals.getPlatformListRef(), arrivals.getBoxListRef(), arrivals.getGuyListRef(), arrivals.getButtonListRef(), nextButtonState);

        // Trigger system execution
        triggerSystem.getPlatformDestinations(nextButtonState, platformDesinations);
    }

    // platforms set their new location and velocity from trigger system data (and ofc their physical data)
    platformStep(arrivals.getPlatformListRef(), nextPlatform, platformDesinations);

    // pickup position update from platform

    // boxes do their crazy wizz-bang collision algorithm
    crappyBoxCollisionAlogorithm(arrivals.getBoxListRef(), nextBox, nextPlatform, time);

    // item simple collision algorithm

    // portal position update
    portalPositionUpdate(nextPlatform, arrivals.getPortalListRef(), nextPortal);

    bool currentPlayerFrame(false);
    TimeDirection currentPlayerDirection(INVALID);
    bool nextPlayerFrame(false);
    bool winFrame(false);

    // guys simple collision algorithm
    guyStep(arrivals.getGuyListRef(), time, playerInput,
            nextGuy, nextBox, nextPlatform, nextPortal, newDepartures, currentPlayerFrame, currentPlayerDirection, nextPlayerFrame, winFrame, pauseTimes);

    // button position update
    buttonPositionUpdate(nextPlatform, nextButtonState, arrivals.getButtonListRef(), nextButton);

    buildDepartures(nextBox, nextPlatform, nextPortal, nextButton, nextGuy,
                    arrivals.getBoxThiefListRef(), arrivals.getBoxExtraListRef(), arrivals.getGuyExtraListRef(),
                    newDepartures, time, pauseTimes);

    //Sort all object lists before returning to other code. They must be sorted for comparisons to work correctly.
    boost::for_each(newDepartures,SortObjectList());
    // add data to departures
    return PhysicsReturnT( newDepartures, currentPlayerFrame, currentPlayerDirection, nextPlayerFrame, winFrame);
}


template <class Type>
void PhysicsEngine::buildDeparturesForComplexEntities(
    const std::vector<ObjectAndTime<Type> >& next,
    const std::vector<const RemoteDepartureEdit<Type>* >& thieves,
    const std::vector<const RemoteDepartureEdit<Type>* >& extras,
    NewDeparturesT& newDepartures,
    Frame* time,
    std::vector<PauseInitiatorID>& pauseTimes) const
{
    // builds departures for something that can move in complex ways throughout pause frames
    // adding, removal etc... things like guys and boxes
    foreach (const ObjectAndTime<Type>& thingAndTime, next)
    {
        const Type& thing(thingAndTime.object);

        // the index of the next normal departure for a thing
        Frame* nextTime(time->nextFrame(thing.getTimeDirection()));

        if (thingAndTime.time != nextTime)
        {
            newDepartures[thingAndTime.time].add(thing);
            goto buildNext;
        }

        // check if the departure is to be stolen
        foreach (const RemoteDepartureEdit<Type>& thief, thieves | boost::adaptors::indirected)
        {
            if (thief.getDeparture() == thing)
            {
                // by now the departure is known to be stolen
                // if the departure is a pause departure the departure a level up must also be stolen
                if (thing.getPauseLevel() != 0)
                {
                    newDepartures[time->parentFrame()].addThief
                    (
                        RemoteDepartureEdit<Type>
                        (
                            time->getInitiatorID(),
                            Type(thing, thing.getTimeDirection(), thing.getPauseLevel()-1),
                            true
                        )
                    );
                }

                // adds pause time departures to pause times before this one
                // also adds pause time departure to the pause time that stole the departue
                foreach (const PauseInitiatorID& pauseTime, pauseTimes)
                {
                    newDepartures[time->entryChildFrame(pauseTime, thing.getTimeDirection())].add
                    (
                        Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
                    );

                    if (pauseTime == thief.getOrigin())
                    {
                        // the current thing is finished, goto next one
                        goto buildNext;
                    }
                }
                assert(false && "pauseTimes must have a element that is equal to thief origin");
            }
        }

        // the depature is not stolen for this thing

        // if the this is a pause time thing and this is the end of the universe do not depart
        if (time->nextFramePauseLevelDifference(thing.getTimeDirection()) != 0 && thing.getPauseLevel() != 0)
        {

        }
        else if (nextTime)
        {
            // simply depart to next frame
            newDepartures[nextTime].add(thing);
        }

        // add pause time departure to all spawned pause times
        foreach (const PauseInitiatorID& pauseTime, pauseTimes)
        {
            newDepartures[time->entryChildFrame(pauseTime, thing.getTimeDirection())].add
            (
                Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
            );
        }
buildNext:
        ;
    }

    // special departures for things, from pause time
    // these things are pause time things that have changed location in pause time
    foreach (const RemoteDepartureEdit<Type>& extra, extras | boost::adaptors::indirected)
    {
        const Type& thing(extra.getDeparture());

        // the index of the next normal departure for a thing
        Frame* nextTime(time->nextFrame(thing.getTimeDirection()));

        // check if the departure is to be stolen
        foreach (const RemoteDepartureEdit<Type>& thief, thieves | boost::adaptors::indirected)
        {
            if (thief.getDeparture() == thing)
            {
                // by now the departure is known to be stolen
                // if the departure is a pause departure the departure a level up must also be stolen
                if (thing.getPauseLevel() != 0)
                {
                    newDepartures[time->parentFrame()].addThief
                    (
                        RemoteDepartureEdit<Type>
                        (
                            time->getInitiatorID(),
                            Type(thing, thing.getTimeDirection(), thing.getPauseLevel()-1),
                            true
                        )
                    );
                }

                // adds pause time departures to pause times before this one
                // also adds pause time departure to the pause time that stole the departue
                // CHANGE FROM NORMAL THING HANDLING: only add to pause times that occur after the extra thing pause time
                foreach (const PauseInitiatorID& pauseTime, pauseTimes)
                {
                    if (extra.getOrigin() < pauseTime)
                    {
                        newDepartures[time->entryChildFrame(pauseTime, thing.getTimeDirection())].add
                        (
                            Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
                        );
                    }

                    if (thief.getOrigin() == pauseTime)
                    {
                        // the thing is finished, goto next one
                        goto buildNextExtra;
                    }
                }
                assert(false && "pauseTimes must have a element that is equal to thief origin");
            }
        }
        // the depature is not stolen for this thing

        // if the thing is a pause time thing and this is the end of the universe do not depart
        if (thing.getPauseLevel() != 0 && time->nextFramePauseLevelDifference(thing.getTimeDirection()) != 0)
        {

        }
        else if (nextTime && extra.getPropIntoNormal())
        {
            // simply depart to next frame
            newDepartures[nextTime].add(thing);
        }

        // CHANGE FROM NORMAL THING HANDLING: add pause time departure to pause times after the current one
        foreach (const PauseInitiatorID& pauseTime, pauseTimes)
        {
            if (extra.getOrigin() < pauseTime)
            {
                newDepartures[time->entryChildFrame(pauseTime, thing.getTimeDirection())].add
                (
                    Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
                );
            }
        }
buildNextExtra:
        ;
    }

}

template <class Type>
void PhysicsEngine::buildDeparturesForReallySimpleThings(
    const std::vector<Type>& next,
    NewDeparturesT& newDepartures,
    Frame* time,
    std::vector<PauseInitiatorID>& pauseTimes
) const
{
    foreach (const Type& thing, next)
    {
        Frame* nextTime(time->nextFrame(thing.getTimeDirection()));

        if (nextTime && (thing.getPauseLevel() == 0 || time->nextFramePauseLevelDifference(thing.getTimeDirection()) == 0))
        {
            newDepartures[nextTime].add(thing);
        }

        foreach (const PauseInitiatorID& pauseTime, pauseTimes)
        {
            newDepartures[time->entryChildFrame(pauseTime, thing.getTimeDirection())].add
            (
                Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
            );
        }
    }
}


void PhysicsEngine::buildDepartures(
    const std::vector<ObjectAndTime<Box> >& nextBox,
    const std::vector<Platform>& nextPlatform,
    const std::vector<Portal>& nextPortal,
    const std::vector<Button>& nextButton,
    const std::vector<ObjectAndTime<Guy> >& nextGuy,
    const std::vector<const RemoteDepartureEdit<Box>* >& boxThieves,
    const std::vector<const RemoteDepartureEdit<Box>* >& extraBoxes,
    const std::vector<const RemoteDepartureEdit<Guy>* >& extraGuys,
    NewDeparturesT& newDepartures,
    Frame* time,
    std::vector<PauseInitiatorID>& pauseTimes) const
{

    // pause times initiated in the frame must be sorted
    boost::sort(pauseTimes);

    buildDeparturesForComplexEntities(nextBox, boxThieves, extraBoxes, newDepartures, time, pauseTimes);

    // build departures for guys
    foreach (const ObjectAndTime<Guy>& guyAndTime, nextGuy)
    {
        const Guy& guyData(guyAndTime.object);

        Frame* nextTime(time->nextFrame(guyData.getTimeDirection()));

        // Depart to next frame but do not depart if the guy is paused and it is the end of a pause time
        if (guyAndTime.time == nextTime)
        {
            if (nextTime && (guyData.getPauseLevel() == 0 || time->nextFramePauseLevelDifference(guyData.getTimeDirection()) == 0))
            {
                newDepartures[nextTime].add(guyData);
            }
        }
        else
        {
            newDepartures[guyAndTime.time].add(guyData);
        }

        // Add extra departures if departing from pause time so that subsequent pause times in the same parent frame see
        // this guy's state at the end of it's pause time. Pause times are ordered.
        if (time->nextFramePauseLevelDifference(guyData.getTimeDirection()) != 0 and guyData.getPauseLevel() == 0 )
        {
            int universes(time->nextFramePauseLevelDifference(guyData.getTimeDirection()));
            Frame* parTime(time);
            do
            {
                const PauseInitiatorID& parInit(parTime->getInitiatorID());
                parTime = parTime->parentFrame();
                // This should be the ONLY place extra guys are added.
                // REMEMBER: PAUSE TIME GUNS DO NOT WORK
                newDepartures[parTime].addExtra
                (
                    RemoteDepartureEdit<Guy>
                    (
                        parInit,
                        Guy(guyData.getX(), guyData.getY(), guyData.getXspeed(), guyData.getYspeed(),
                            guyData.getWidth(),guyData.getHeight(), guyData.getRelativeToPortal(), guyData.getSupported(),
                            guyData.getBoxCarrying(), guyData.getBoxCarrySize(),
                            guyData.getBoxCarryDirection(), guyData.getBoxPauseLevel(),
                            guyData.getTimeDirection(), guyData.getPauseLevel()+1,
                            std::numeric_limits<std::size_t>::max()
                           ),
                        false
                    )

                );
                --universes;
            }
            while (universes > 0);
        }


        if (guyAndTime.time->parentFrame() == time)
        {
            // if the guy is departing to paused don't add it to pause times after this one
            const PauseInitiatorID& pauseID(guyAndTime.time->getInitiatorID());
            foreach (const PauseInitiatorID& pauseTime, pauseTimes)
            {
                if (pauseID == pauseTime)
                {
                    break;
                }
                newDepartures[time->entryChildFrame(pauseTime, guyData.getTimeDirection())].add(
                    Guy(guyData.getX(), guyData.getY(), guyData.getXspeed(), guyData.getYspeed(),
                        guyData.getWidth(),guyData.getHeight(), guyData.getRelativeToPortal(), guyData.getSupported(),
                        guyData.getBoxCarrying(), guyData.getBoxCarrySize(),
                        guyData.getBoxCarryDirection(), guyData.getBoxPauseLevel(),
                        guyData.getTimeDirection(), guyData.getPauseLevel()+1,
                        std::numeric_limits<std::size_t>::max()));
            }
        }
        else
        {
            // add pause guy to every pause time universe from this frame
            foreach (const PauseInitiatorID& pauseTime, pauseTimes)
            {
                newDepartures[time->entryChildFrame(pauseTime, guyData.getTimeDirection())].add(
                    Guy(guyData.getX(), guyData.getY(), guyData.getXspeed(), guyData.getYspeed(),
                        guyData.getWidth(),guyData.getHeight(), guyData.getRelativeToPortal(), guyData.getSupported(),
                        guyData.getBoxCarrying(), guyData.getBoxCarrySize(),
                        guyData.getBoxCarryDirection(), guyData.getBoxPauseLevel(),
                        guyData.getTimeDirection(), guyData.getPauseLevel()+1,
                        std::numeric_limits<std::size_t>::max()));
            }
        }
    }

    // build departure for extra guys (purely graphical things to do with pause order)
    foreach (const RemoteDepartureEdit<Guy>& extraGuy, extraGuys | boost::adaptors::indirected)
    {
        // add pause time departure to pause times after the current one
        foreach (const PauseInitiatorID& pauseTime, pauseTimes | boost::adaptors::reversed)
        {
            if (extraGuy.getOrigin() < pauseTime)
            {
                newDepartures[time->entryChildFrame(pauseTime, extraGuy.getDeparture().getTimeDirection())].add
                (
                    Guy(extraGuy.getDeparture())
                );
            }
            else
            {
                //pauseTimes is sorted (in ascending order)
                break;
            }
        }
    }

    // simple things
    buildDeparturesForReallySimpleThings(nextPlatform, newDepartures, time, pauseTimes);
    buildDeparturesForReallySimpleThings(nextButton, newDepartures, time, pauseTimes);
    buildDeparturesForReallySimpleThings(nextPortal, newDepartures, time, pauseTimes);
}

void PhysicsEngine::guyStep(const std::vector<const Guy*>& oldGuyList,
                            Frame* time,
                            const std::vector<InputList>& playerInput,
                            std::vector<ObjectAndTime<Guy> >& nextGuy,
                            std::vector<ObjectAndTime<Box> >& nextBox,
                            const std::vector<Platform>& nextPlatform,
                            const std::vector<Portal>& nextPortal,
                            NewDeparturesT& newDepartures,
                            bool& currentPlayerFrame,
                            TimeDirection& currentPlayerDirection,
                           	bool& nextPlayerFrame,
                            bool& winFrame,
                            std::vector<PauseInitiatorID>& pauseTimes) const
{
    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> xspeed;
    std::vector<int> yspeed;
    std::vector<char> supported;

    x.reserve(oldGuyList.size());
    y.reserve(oldGuyList.size());
    xspeed.reserve(oldGuyList.size());
    yspeed.reserve(oldGuyList.size());
    supported.reserve(oldGuyList.size());

    // position, velocity, collisions
    for (std::size_t i = 0; i < oldGuyList.size(); ++i)
    {
        if (oldGuyList[i]->getRelativeToPortal() == -1)
        {
            x.push_back(oldGuyList[i]->getX());
            y.push_back(oldGuyList[i]->getY());
        }
        else
        {
            Portal relativePortal(nextPortal[oldGuyList[i]->getRelativeToPortal()]);
            x.push_back(relativePortal.getX() + oldGuyList[i]->getX());
            y.push_back(relativePortal.getY() + oldGuyList[i]->getY());
        }
        xspeed.push_back(0);
        yspeed.push_back(oldGuyList[i]->getYspeed() + gravity);
        supported.push_back(false);

        if (oldGuyList[i]->getIndex() < playerInput.size() && oldGuyList[i]->getPauseLevel() == 0u)
        {
            std::size_t relativeIndex(oldGuyList[i]->getIndex());
            const InputList& input = playerInput[relativeIndex];

            int width = oldGuyList[i]->getWidth();
            int height = oldGuyList[i]->getHeight();

            // jump
            if (oldGuyList[i]->getSupported() && input.getUp())
            {
                yspeed[i] = -550;
            }

            //check wall collision in Y direction
            int newY = y[i] + yspeed[i];

            if (yspeed[i] > 0) // down
            {
                if (wallAt(x[i], newY+height) || (x[i] - (x[i]/wallSize)*wallSize > wallSize-width && wallAt(x[i]+width, newY+height)))
                {
                    newY = ((newY+height)/wallSize)*wallSize - height;
                    supported[i] = true;
                }
            }
            else if (yspeed[i] < 0) // up
            {
                if  (wallAt(x[i], newY) || (x[i] - (x[i]/wallSize)*wallSize > wallSize-width && wallAt(x[i]+width, newY)))
                {
                    newY = (newY/wallSize + 1)*wallSize;
                }
            }

            // platform collision
            foreach (const Platform& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection * oldGuyList[i]->getTimeDirection() == hg::REVERSE)
                {
                    pX -= platform.getXspeed();
                    pY -= platform.getYspeed();
                }
                int pWidth(platform.getWidth());
                int pHeight(platform.getHeight());

                if (IntersectingRectanglesExclusive(x[i], newY, width, height, pX, pY, pWidth, pHeight))
                {
                    if (newY+height/2 < pY+pHeight/2)
                    {
                        newY = pY-height;
                        xspeed[i] = pDirection * oldGuyList[i]->getTimeDirection() * platform.getXspeed();
                        supported[i] = true;
                    }
                    else
                    {
                        newY = pY + pHeight;
                    }
                }
            }

            // box collision
            foreach (const ObjectAndTime<Box>& box, nextBox)
            {
                int boxX(box.object.getX());
                int boxY(box.object.getY());
                int boxXspeed(box.object.getXspeed());
                int boxYspeed(box.object.getYspeed());
                int boxSize(box.object.getSize());
                TimeDirection boxDirection(box.object.getTimeDirection());
                if (x[i] <= boxX+boxSize && x[i]+width >= boxX)
                {
                    if (box.object.getPauseLevel() > 0)
                    {
                        if (newY+height >= boxY && newY+height-yspeed[i] <= boxY)
                        {
                            newY = boxY-height;
                            xspeed[i] = 0;
                            supported[i] = true;
                        }
                    }
                    else if (boxDirection*oldGuyList[i]->getTimeDirection() == hg::REVERSE)
                    {
                        if (newY+height >= boxY-boxYspeed && newY+height-yspeed[i] <= boxY)
                        {
                            newY = boxY-height-boxYspeed;
                            xspeed[i] = -boxXspeed;
                            supported[i] = true;
                        }

                    }
                    else
                    {
                        if (newY+height >= boxY && newY-yspeed[i]+height <= boxY-boxYspeed)
                        {
                            newY = boxY-height;
                            xspeed[i] = boxXspeed;
                            supported[i] = true;
                        }
                    }
                }
            }


            //check wall collision in X direction
            int newX(x[i] + xspeed[i]);

            if (input.getLeft())
            {
                newX += -250;
            }
            else if (input.getRight())
            {
                newX += 250;
            }

            if (newX-x[i] > 0) // right
            {
                if (wallAt(newX+width, newY) || (newY - (newY/wallSize)*wallSize > wallSize-height && wallAt(newX+width, newY+height)))
                {
                    newX = (newX+width)/wallSize*wallSize - width;
                }
            }
            else if (newX-x[i] < 0) // left
            {
                if (wallAt(newX, newY) || (newY - (newY/wallSize)*wallSize > wallSize-height && wallAt(newX, newY+height)))
                {
                    newX = (newX/wallSize + 1)*wallSize;
                }
            }

            // platform collision
            foreach (const Platform& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection*oldGuyList[i]->getTimeDirection() == hg::REVERSE)
                {
                    pX -= platform.getXspeed();
                    pY -= platform.getYspeed();
                }
                int pWidth = platform.getWidth();
                int pHeight = platform.getHeight();

                if (IntersectingRectanglesExclusive(newX, newY, width, height, pX, pY, pWidth, pHeight))
                {
                    if (newX+width/2 < pX+pWidth/2)
                    {
                        newX = pX-width;
                    }
                    else
                    {
                        newX = pX+pWidth;
                    }
                }
            }

            xspeed[i] = newX-x[i];
            yspeed[i] = newY-y[i];

            x[i] = newX;
            y[i] = newY;
        }
    }

    std::vector<char> carry;
    std::vector<int> carrySize;
    std::vector<TimeDirection> carryDirection;
    std::vector<int> carryPauseLevel;
    carry.reserve(oldGuyList.size());
    carrySize.reserve(oldGuyList.size());
    carryDirection.reserve(oldGuyList.size());
    carryPauseLevel.reserve(oldGuyList.size());
    // box carrying
    for (std::size_t i = 0; i < oldGuyList.size(); ++i)
    {

        carry.push_back(oldGuyList[i]->getBoxCarrying());
        carrySize.push_back(0);
        carryDirection.push_back(hg::INVALID);
        carryPauseLevel.push_back(0);

        if (oldGuyList[i]->getIndex() < playerInput.size() && oldGuyList[i]->getPauseLevel() == 0u)
        {

            std::size_t relativeIndex(oldGuyList[i]->getIndex());
            const InputList& input = playerInput[relativeIndex];

            if (carry[i])
            {
                bool droppable = false;
                if (input.getDown())
                {
                    int width(oldGuyList[i]->getWidth());
                    int dropX(x[i] + width/2 - oldGuyList[i]->getBoxCarrySize()/2);
                    int dropY(y[i] - oldGuyList[i]->getBoxCarrySize());
                    int dropSize(oldGuyList[i]->getBoxCarrySize());

                    if (!wallAt(dropX, dropY, dropSize, dropSize))
                    {
                        droppable = true;
                        for (unsigned int j = 0; droppable && j < nextBox.size(); ++j)
                        {
                            if (IntersectingRectanglesExclusive(
                                    nextBox[j].object.getX(), nextBox[j].object.getY(),
                                    nextBox[j].object.getSize(), nextBox[j].object.getSize(),
                                    dropX, dropY, dropSize, dropSize))
                            {
                                droppable = false;
                            }
                        }
                        for (unsigned int j = 0; droppable && j < nextPlatform.size(); ++j)
                        {
                            if (IntersectingRectanglesExclusive(nextPlatform[j].getX(), nextPlatform[j].getY(),
                                                                nextPlatform[j].getWidth(), nextPlatform[j].getHeight(),
                                                                dropX, dropY, dropSize, dropSize))
                            {
                                droppable = false;
                                break;
                            }
                        }
                        if (droppable)
                        {
                            nextBox.push_back
                            (
                                ObjectAndTime<Box>
                                (
                                    Box
                                    (
                                        dropX,
                                        dropY,
                                        0,
                                        0,
                                        dropSize,
                                        oldGuyList[i]->getBoxCarryDirection(),
                                        oldGuyList[i]->getBoxPauseLevel()
                                    ),
                                    time->nextFrame(oldGuyList[i]->getBoxCarryDirection())//,
                                    //false
                                )
                            );

                            if (oldGuyList[i]->getBoxPauseLevel() != 0)
                            {
                                int pauseLevel = oldGuyList[i]->getBoxPauseLevel();
                                Frame* parTime = time;
                                int pauseLevelChange = 1;
                                while (pauseLevel > 0)
                                {
                                    PauseInitiatorID parInit(parTime->getInitiatorID());
                                    parTime = parTime->parentFrame();
                                    newDepartures[parTime].addExtra
                                    (
                                        RemoteDepartureEdit<Box>
                                        (
                                            parInit,
                                            Box
                                            (
                                                dropX, dropY, 0, 0,
                                                dropSize, oldGuyList[i]->getBoxCarryDirection(),
                                                oldGuyList[i]->getBoxPauseLevel()-pauseLevelChange
                                            ),
                                            true
                                        )

                                    );
                                    --pauseLevel;
                                    ++pauseLevelChange;
                                }
                            }

                            carry[i] = false;
                            carrySize[i] = 0;
                            carryDirection[i] = hg::INVALID;
                            carryPauseLevel[i] = 0;
                        }
                    }
                }

                if (!droppable)
                {
                    carrySize[i] = oldGuyList[i]->getBoxCarrySize();
                    carryDirection[i] = oldGuyList[i]->getBoxCarryDirection();
                    carryPauseLevel[i] = oldGuyList[i]->getBoxPauseLevel();
                }
            }
            else
            {
                if (input.getDown())
                {
                    int width = oldGuyList[i]->getWidth();
                    int height = oldGuyList[i]->getHeight();

                    for (unsigned int j = 0; j < nextBox.size(); ++j)
                    {
                        int boxX = nextBox[j].object.getX();
                        int boxY = nextBox[j].object.getY();
                        int boxSize = nextBox[j].object.getSize();
                        if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY))
                        {
                            carry[i] = true;
                            carrySize[i] = boxSize;
                            carryDirection[i] = nextBox[j].object.getTimeDirection();
                            carryPauseLevel[i] = nextBox[j].object.getPauseLevel();
                            if (nextBox[j].object.getPauseLevel() != 0)
                            {
                                newDepartures[time->parentFrame()].addThief
                                (
                                    RemoteDepartureEdit<Box>
                                    (
                                        time->getInitiatorID(),
                                        Box
                                        (
                                            boxX, boxY, nextBox[j].object.getXspeed(), nextBox[j].object.getYspeed(),
                                            boxSize, carryDirection[i], carryPauseLevel[i]-1
                                        ),
                                        true
                                    )
                                );
                            }
                            nextBox.erase(nextBox.begin() + j);
                            break;
                        }
                    }
                }
                else
                {
                    carrySize[i] = 0;
                    carryDirection[i] = hg::INVALID;
                    carryPauseLevel[i] = 0;
                }
            }
        }
    }
    // colliding with pickups?
    // time travel
    for (std::size_t i(0), size(oldGuyList.size()); i != size; ++i)
    {
        if (oldGuyList[i]->getPauseLevel() != 0)
        {
            nextGuy.push_back(
                ObjectAndTime<Guy>(
                    *oldGuyList[i],
                    time->nextFrame(oldGuyList[i]->getTimeDirection())));
        }
        else if (oldGuyList[i]->getIndex() < playerInput.size())
        {
            std::size_t relativeIndex(oldGuyList[i]->getIndex());
            const InputList& input = playerInput[relativeIndex];
            int nextCarryPauseLevel = carryPauseLevel[i];
            int relativeToPortal = -1;

            // add departure for guy at the appropriate time
            TimeDirection nextTimeDirection = oldGuyList[i]->getTimeDirection();
            Frame* nextTime(time->nextFrame(nextTimeDirection));
            assert(time);

            bool normalDeparture = true;

            if (input.getAbility() == hg::TIME_JUMP)
            {
                nextTime = time->arbitraryFrameInUniverse(input.getFrameIdParam(0).getFrameNumber());
                normalDeparture = false;
            }
            else if (input.getAbility() == hg::TIME_REVERSE)
            {
                normalDeparture = false;
                nextTimeDirection *= -1;
                nextTime = time->nextFrame(nextTimeDirection);
                carryDirection[i] *= -1;

                if (time->nextFramePauseLevelDifference(nextTimeDirection) != 0)
                {
                    nextCarryPauseLevel -= time->nextFramePauseLevelDifference(nextTimeDirection);
                    if (nextCarryPauseLevel < 0)
                    {
                        nextCarryPauseLevel = 0;
                    }
                }
            }
            else if (input.getAbility() == hg::PAUSE_TIME and time->getFrameNumber() == 3000) // FOR TESTING, REMOVE
            {
                normalDeparture = false;
                PauseInitiatorID pauseID = PauseInitiatorID(pauseinitiatortype::GUY, relativeIndex, 500);
                nextTime = time->entryChildFrame(pauseID, oldGuyList[i]->getTimeDirection());

                pauseTimes.push_back(pauseID);
            }
            else if (input.getUse() == true)
            {
                int mx = x[i] + oldGuyList[i]->getWidth();
                int my = y[i] + oldGuyList[i]->getHeight();
                for (unsigned int j = 0; j < nextPortal.size(); ++j)
                {
                    int px = nextPortal[j].getX();
                    int py = nextPortal[j].getY();
                    int pw = nextPortal[j].getWidth();
                    int ph = nextPortal[j].getHeight();
                    if (PointInRectangleInclusive(mx, my, px, py, pw, ph) && nextPortal[j].getPauseLevel() == 0 &&
                            nextPortal[j].getActive() && nextPortal[j].getCharges() != 0) // charges not fully implemented
                    {
                        Frame* portalTime;
                        if (nextPortal[j].getWinner())
                        {
                        	winFrame = true;
                        	nextTime = 0;
                        	break;
                        }

                        if (nextPortal[j].getRelativeTime() == true)
                        {
                            portalTime = time->arbitraryFrameInUniverse(time->getFrameNumber() + nextPortal[j].getTimeDestination());
                        }
                        else
                        {
                            portalTime = time->arbitraryFrameInUniverse(nextPortal[j].getTimeDestination());
                        }
                        if (portalTime)
                        {
                            nextTime = portalTime;
                            normalDeparture = false;
                            relativeToPortal = nextPortal[j].getDestinationIndex();
                            x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination();
                            y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination();
                        }
                    }
                }
            }

            if (normalDeparture)
            {
                if (time->nextFramePauseLevelDifference(nextTimeDirection) != 0)
                {
                    nextCarryPauseLevel -= time->nextFramePauseLevelDifference(nextTimeDirection);
                    if (nextCarryPauseLevel < 0)
                    {
                        nextCarryPauseLevel = 0;
                    }
                }
            }

            if (playerInput.size() - 1 == relativeIndex && nextTime)
            {
                currentPlayerDirection = oldGuyList[i]->getTimeDirection();
                currentPlayerFrame = true;
                //cout << "nextPlayerFrame set to: " << nextPlayerFrame.frame() << "  " << x[i] << "\n";
            }

            if (nextTime)
            {
                nextGuy.push_back(
                    ObjectAndTime<Guy>(
                        Guy(
                            x[i], y[i], xspeed[i], yspeed[i],
                            oldGuyList[i]->getWidth(), oldGuyList[i]->getHeight(),
                            relativeToPortal, supported[i],
                            carry[i], carrySize[i], carryDirection[i], nextCarryPauseLevel,
                            nextTimeDirection, 0,
                            relativeIndex+1),
                        nextTime));
            }
        }
        else
        {
            assert(oldGuyList[i]->getIndex() == playerInput.size());
            nextPlayerFrame = true;
        }
    }
}

void PhysicsEngine::crappyBoxCollisionAlogorithm(
    const std::vector<const Box*>& oldBoxList,
    std::vector<ObjectAndTime<Box> >& nextBox,
    std::vector<Platform>& nextPlatform,
    Frame* time) const
{
    for (std::vector<const Box*>::const_iterator i(oldBoxList.begin()), iend(oldBoxList.end()); i != iend; ++i)
    {
        if ((*i)->getPauseLevel() != 0)
        {
            nextBox.push_back
            (
                ObjectAndTime<Box>
                (
                    **i,
                    time->nextFrame((*i)->getTimeDirection())//,
                    //true
                )
            );

            continue;
        }

        int x = (*i)->getX();
        int y = (*i)->getY();
        int xspeed = (*i)->getXspeed();
        int yspeed = (*i)->getYspeed() + gravity;
        int size = (*i)->getSize();

        bool supported = false;

        bool exploded = false;
        /*
        for (vector<Box>::const_iterator j(oldBoxList.begin()), jend(oldBoxList.end()); j != jend; ++j)
        {
            if (j != i)
            {
                int boxX = j->getX();
                int boxY = j->getY();
                int boxSize = j->getSize();
                if (PhysicsEngine::IntersectingRectanglesExclusive(x, y, size, size, boxX, boxY, boxSize, boxSize))
                {
                    exploded = true;
                    continue;
                }
            }
        }
        */
        /*
        for (vector<Platform>::const_iterator j(nextPlatform.begin()), jend(nextPlatform.end()); j != jend; ++j)
        {
            if (PhysicsEngine::IntersectingRectanglesExclusive(x, y, size, size, j->getX(), j->getY(), j->getHeight(), j->getWidth()))
            {
                exploded = true;
                continue;
            }
        }
        */
        if (exploded)
        {
            continue;
        }

        // ** Y component **
        int newY = y + yspeed;

        // box collision
        for (std::vector<const Box*>::const_iterator j(oldBoxList.begin()), jend(oldBoxList.end()); j != jend; ++j)
        {
            if (j != i)
            {
                int boxX = (*j)->getX();
                int boxY = (*j)->getY();
                int boxSize = (*j)->getSize();
                if (IntersectingRectanglesInclusive(x, newY, size, size, boxX, boxY, boxSize, boxSize))
                {
                    if (newY + size/2 < boxY + boxSize/2)
                    {
                        newY = boxY - size;
                    }
                    else
                    {
                        //newY = boxY + boxSize;
                    }
                }
            }
        }
        yspeed = newY - y;

        //check wall collision in Y direction
        if (yspeed > 0) // down
        {
            if (wallAt(x, newY+size)/*wallmap[x/wallSize][(newY+size)/wallSize]*/ || (x - (x/wallSize)*wallSize > wallSize-size && wallAt(x+size,newY+size)/*wallmap[(x+size)/wallSize][(newY+size)/wallSize]*/))
            {
                newY = ((newY+size)/wallSize)*wallSize - size;
                supported = true;
                xspeed = 0;
            }
        }
        else if (yspeed < 0) // up
        {
            if  (wallAt(x, newY)/*wallmap[x/wallSize][newY/wallSize]*/ || (x - (x/wallSize)*wallSize > wallSize-size && wallAt(x+size, newY) /*wallmap[(x+size)/wallSize][newY/wallSize]*/))
            {
                newY = (newY/wallSize + 1)*wallSize;
            }
        }

        // platform collision
        foreach (const Platform& platform, nextPlatform) {
            int pX(platform.getX());
            int pY(platform.getY());
            TimeDirection pDirection = platform.getTimeDirection();
            if (pDirection * (*i)->getPauseLevel() == 0 && pDirection*(*i)->getTimeDirection() == hg::REVERSE)
            {
                pX -= platform.getXspeed();
                pY -= platform.getYspeed();
            }

            int pWidth(platform.getWidth());
            int pHeight(platform.getHeight());
            if (IntersectingRectanglesExclusive(x, newY, size, size, pX, pY, pWidth, pHeight)) {
                if (newY + size/2 < pY + pHeight/2) {
                    newY = pY-size;
                    xspeed = pDirection * (*i)->getTimeDirection() * platform.getXspeed();
                    supported = true;
                }
                else {
                    newY = pY+pHeight;
                }
            }
        }

        // ** X component **
        int newX = x + xspeed;

        for (std::vector<const Box*>::const_reverse_iterator j(oldBoxList.rbegin()), jend(oldBoxList.rend()); j != jend; ++j)
        {
            if ((j.base() - 1) != i )
            {
                {
                    int boxX = (*j)->getX();
                    int boxY = (*j)->getY();
                    int boxSize = (*j)->getSize();
                    if (IntersectingRectanglesInclusive(newX, newY, size, size, boxX, boxY, boxSize, boxSize))
                    {
                        if (newX + size/2 < boxX + boxSize/2)
                        {
                            newX = x + (*j)->getXspeed()/10;//boxX - size;
                        }
                        else
                        {
                            newX = x + (*j)->getXspeed()/10;//boxX + boxSize;
                        }
                    }
                }
            }
        }
        xspeed = newX - x;

        //check wall collision in X direction
        if (xspeed > 0) // right
        {
            if (wallmap[(newX+size)/wallSize][newY/wallSize] || (newY - (newY/wallSize)*wallSize > wallSize-size && wallmap[(newX+size)/wallSize][(newY+size)/wallSize]))
            {
                newX = (newX+size)/wallSize*wallSize - size;
            }
        }
        else if (xspeed < 0) // left
        {
            if (wallmap[newX/wallSize][newY/wallSize] || (newY - (newY/wallSize)*wallSize > wallSize-size && wallmap[newX/wallSize][(newY+size)/wallSize]))
            {
                newX = (newX/wallSize + 1)*wallSize;
            }
        }

        // platform collision
        for (unsigned int j = 0; j < nextPlatform.size(); ++j)
        {
            int pX = nextPlatform[j].getX();
            int pY = nextPlatform[j].getY();
            TimeDirection pDirection = nextPlatform[j].getTimeDirection();
            if (pDirection*(*i)->getTimeDirection() == hg::REVERSE)
            {
                pX -= nextPlatform[j].getXspeed();
                pY -= nextPlatform[j].getYspeed();
            }
            int pWidth = nextPlatform[j].getWidth();
            int pHeight = nextPlatform[j].getHeight();

            if (IntersectingRectanglesExclusive(newX, newY, size, size, pX, pY, pWidth, pHeight))
            {
                if (newX+size/2 < pX+pWidth/2)
                {
                    newX = pX-size;
                }
                else
                {
                    newX = pX+pWidth;
                }
            }
        }

        xspeed = newX-x;
        yspeed = newY-y;

        x = newX;
        y = newY;

        nextBox.push_back
        (
            ObjectAndTime<Box>(
                Box(
                    x,
                    y,
                    xspeed,
                    yspeed,
                    size,
                    (*i)->getTimeDirection(),
                    0),
                time->nextFrame((*i)->getTimeDirection())//,
                //supported
            )
        );

    }
}

void PhysicsEngine::platformStep(const std::vector<const Platform*>& oldPlatformList,
                                 std::vector<Platform>& nextPlatform,
                                 const std::vector<PlatformDestination>& pd) const
{
    if (boost::distance(pd) == 0) {
        boost::push_back(nextPlatform, oldPlatformList | boost::adaptors::indirected);
        return;
    }

    assert(boost::distance(oldPlatformList) <= boost::distance(pd));
    boost::range_iterator<boost::indirected_range<const std::vector<const Platform*> > >::type
    platIt(boost::begin(oldPlatformList | boost::adaptors::indirected)),
    platEnd(boost::end(oldPlatformList | boost::adaptors::indirected));
    boost::range_iterator<const std::vector<PlatformDestination> >::type
    destIt(boost::begin(pd));
    for (; platIt != platEnd; ++platIt, ++destIt)
    {
        const Platform& platform(*platIt);
        const PlatformDestination& destination(*destIt);
        assert(platform.getPauseLevel() == 0);
        int x(platform.getX());
        int y(platform.getY());
        int xspeed(platform.getXspeed());
        int yspeed(platform.getYspeed());

        // X component
        if (destination.getX() != x)
        {
            if (abs(destination.getX() - x) <= abs(xspeed) && abs(xspeed) <= destination.getXdeccel())
            {
                xspeed = destination.getX() - x;
            }
            else
            {
                int direction = abs(x - destination.getX())/(x - destination.getX());

                if (xspeed*direction >= 0)
                {
                    xspeed -= direction * destination.getXdeccel();
                    if (xspeed * direction < 0)
                    {
                        xspeed = 0;
                    }
                }
                else
                {
                    // if the platform can still stop if it fully accelerates
                    if (
                        abs(x - destination.getX())
                        >
                        (static_cast<int>(
                             pow(
                                 static_cast<double>
                                 (xspeed - direction*destination.getXaccel()),2))*3/(2*destination.getXdeccel())))
                    {
                        // fully accelerate
                        xspeed -= direction*destination.getXaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(x - destination.getX()) > (static_cast<int>(pow(static_cast<double>(xspeed),2))*3/(2*destination.getXdeccel())))
                    {
                        // set speed to required speed
                        xspeed = -direction*static_cast<int>(floor(static_cast<double>(abs(x - destination.getX())*destination.getXdeccel()*2/3)));
                    }
                    else
                    {
                        xspeed += direction*destination.getXdeccel();
                    }
                }
            }
        }
        else
        {
            if (abs(xspeed) <= destination.getXdeccel())
            {
                xspeed = 0;
            }
            else
            {
                xspeed += abs(xspeed)/xspeed*destination.getXdeccel();
            }
        }

        if (abs(xspeed) > destination.getXspeed())
        {
            xspeed = abs(xspeed)/xspeed*destination.getXspeed();
        }

        x += xspeed;

        // Y component
        if (destination.getY() != y)
        {
            if (abs(destination.getY() - y) <= abs(yspeed) && abs(yspeed) <= destination.getYdeccel())
            {
                yspeed = destination.getY() - y;
            }
            else
            {
                int direction = abs(y - destination.getY())/(y - destination.getY());

                if (yspeed * direction > 0)
                {
                    yspeed -= direction * destination.getYdeccel();
                    if (yspeed * direction < 0)
                    {
                        yspeed = 0;
                    }
                }
                else
                {
                    // if the platform can still stop if it fully accelerates
                    if (abs(y - destination.getY())
                            >
                            (static_cast<int>(pow(static_cast<double>(yspeed - direction*destination.getYaccel()),2))
                             *3/(2*destination.getYdeccel())))
                    {
                        // fully accelerate
                        yspeed -= direction*destination.getYaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(y - destination.getY()) > (static_cast<int>(pow(static_cast<double>(yspeed),2))*3/(2*destination.getYdeccel())))
                    {
                        // set speed to required speed
                        yspeed = -direction*static_cast<int>(floor(sqrt(static_cast<double>(abs(y - destination.getY())*destination.getYdeccel()*2/3))));
                    }
                    else
                    {
                        yspeed += direction*destination.getYdeccel();
                    }
                }
            }
        }
        else
        {
            if (abs(yspeed) <= destination.getYdeccel())
            {
                yspeed = 0;
            }
            else
            {
                yspeed += abs(yspeed)/yspeed*destination.getYdeccel();
            }
        }

        if (abs(yspeed) > destination.getYspeed())
        {
            yspeed = abs(yspeed)/yspeed*destination.getYspeed();
        }

        y += yspeed;

        // send information
        nextPlatform.push_back(Platform(x, y,
                                        xspeed, yspeed,
                                        platform.getWidth(), platform.getHeight(),
                                        platform.getIndex(),
                                        platform.getTimeDirection(),
                                        0));
    }
}

void PhysicsEngine::portalPositionUpdate(
    const std::vector<Platform>& nextPlatform,
    const std::vector<const Portal*>& oldPortalList,
    std::vector<Portal>& nextPortal) const
{
    const std::vector<Attachment>& attachments(attachmentMap.getPortalAttachmentRef());

    foreach(const Portal& portal, oldPortalList | boost::adaptors::indirected)
    {
        if (portal.getPauseLevel() != 0)
        {
            nextPortal.push_back(portal);
            continue;
        }

        int x(portal.getX());
        int y(portal.getY());

        const Attachment& attachment(attachments[portal.getIndex()]);
        std::size_t pid(attachment.platformIndex);
        if (pid != std::numeric_limits<std::size_t>::max())
        {
            const Platform& platform(nextPlatform[pid]);
            if (nextPlatform[pid].getTimeDirection() * platform.getTimeDirection() == hg::FORWARDS)
            {
                x = platform.getX() + attachment.xOffset;
                y = platform.getY() + attachment.yOffset;
            }
            else
            {
                x = platform.getX() - platform.getXspeed() + attachment.xOffset;
                y = platform.getY() - platform.getYspeed() + attachment.yOffset;
            }
        }

        nextPortal.push_back(Portal(x, y,
                                    x-portal.getX(), y-portal.getY(),
                                    portal.getWidth(), portal.getHeight(),
                                    portal.getIndex(),
                                    portal.getTimeDirection(),
                                    portal.getPauseLevel(),
                                    portal.getCharges(),
                                    portal.getActive(),
                                    portal.getXdestination(),
                                    portal.getYdestination(),
                                    portal.getDestinationIndex(),
                                    portal.getTimeDestination(),
                                    portal.getRelativeTime(),
                                    portal.getWinner()));
    }
}

void PhysicsEngine::buttonPositionUpdate(
    const std::vector<Platform>& nextPlatform,
    const std::vector<char>& nextButtonState,
    const std::vector<const Button*>& oldButtonList,
    std::vector<Button>& nextButton) const
{
    const std::vector<Attachment>& attachments(attachmentMap.getButtonAttachmentRef());
    //This is the minimal assert which ensures defined behaviour for the following code.
    //There should probably be a stricter == assert, but the current code
    //would fail such an assert during level construction.
    //TODO -- look deeper into why this is and whether or not it is desireable/fixable
    assert(boost::distance(oldButtonList) <= boost::distance(nextButtonState));

    for (std::size_t i(0), size(oldButtonList.size()); i != size; ++i)
    {
        if (oldButtonList[i]->getPauseLevel() != 0)
        {
            nextButton.push_back(*oldButtonList[i]);
            continue;
        }

        int x(oldButtonList[i]->getX());
        int y(oldButtonList[i]->getY());
        std::size_t index(oldButtonList[i]->getIndex());
        Attachment attachment(attachments[index]);
        std::size_t pid(attachment.platformIndex);
        if (attachment.platformIndex != std::numeric_limits<std::size_t>::max())
        {
            const Platform& platform(nextPlatform[pid]);
            if (platform.getTimeDirection() * oldButtonList[index]->getTimeDirection() == hg::FORWARDS)
            {
                x = platform.getX() + attachment.xOffset;
                y = platform.getY() + attachment.yOffset;
            }
            else
            {
                x = platform.getX() - platform.getXspeed() + attachment.xOffset;
                y = platform.getY() - platform.getYspeed() + attachment.yOffset;
            }
        }

        nextButton.push_back(
            Button(x, y,
                   x - oldButtonList[i]->getX(), y - oldButtonList[i]->getY(),
                   oldButtonList[i]->getWidth(), oldButtonList[i]->getHeight(),
                   index,
                   nextButtonState[i],
                   oldButtonList[i]->getTimeDirection(),
                   0));
    }
}


void PhysicsEngine::buttonChecks(
    const std::vector<const Platform*>& oldPlatformList,
    const std::vector<const Box*>& oldBoxList,
    const std::vector<const Guy*>& oldGuyList,
    const std::vector<const Button*>& oldButtonList,
    std::vector<char>& nextButton) const
{
    const std::vector<Attachment>& attachments(attachmentMap.getButtonAttachmentRef());
    foreach (const Button& oldButton, oldButtonList | boost::adaptors::indirected)
    {
        int x(oldButton.getX());
        int y(oldButton.getY());
        int w(oldButton.getWidth());
        int h(oldButton.getHeight());
        bool state(false);
        {
            std::size_t index(oldButton.getIndex());
            const Attachment& attachment(attachments[index]);
            std::size_t pid(attachment.platformIndex);
            if (pid != std::numeric_limits<std::size_t>::max())
            {
                const Platform& platform(*oldPlatformList[pid]);
                if (platform.getTimeDirection() * oldButtonList[index]->getTimeDirection() == FORWARDS)
                {
                    x = platform.getX() + attachment.xOffset;
                    y = platform.getY() + attachment.yOffset;
                }
                else
                {
                    x = platform.getX() - platform.getXspeed() + attachment.xOffset;
                    y = platform.getY() - platform.getYspeed() + attachment.yOffset;
                }
            }
        }
        foreach (const Box& box, oldBoxList | boost::adaptors::indirected)
        {
            if (state) {
                break;
            }
            state = IntersectingRectanglesInclusive(x, y, w, h,
                                                    box.getX(), box.getY(),
                                                    box.getSize(), box.getSize());
        }
        foreach (const Guy& guy, oldGuyList | boost::adaptors::indirected)
        {
            if (state) {
                break;
            }
            state = IntersectingRectanglesInclusive(x, y, w, h,
                                                    guy.getX(), guy.getY(),
                                                    guy.getWidth(), guy.getHeight());
        }
        nextButton[oldButton.getIndex()] = state;
    }
}

bool PhysicsEngine::wallAt(int x, int y) const
{
    if (x < 0 || y < 0)
    {
        return true;
    }

    unsigned int aX(x/wallSize);
    unsigned int aY(y/wallSize);

    if (aX < wallmap.size() && aY < wallmap[aX].size())
    {
        return wallmap[aX][aY];
    }
    else
    {
        return true;
    }
}

bool PhysicsEngine::wallAt(int x, int y, int w, int h) const
{
    return wallAt(x, y) || wallAt(x+w, y) || wallAt(x, y+h) || wallAt(x+w, y+h);
}

static bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h)
{
    return
        (px <= x + w && px >= x)
        &&
        (py <= y + h && py >= y)
        ;
}

static bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
        (
            (x1 <= x2 && x1 + w1 >= x2)
            ||
            (x2 <= x1 && x2 + w2 >= x1)
        )
        &&
        (
            (y1 <= y2 && y1 + h1 >= y2)
            ||
            (y2 <= y1 && y2 + h2 >= y1)
        )
        ;
}

static bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
        (
            (x1 < x2 && x1 + w1 > x2)
            ||
            (x2 < x1 && x2 + w2 > x1)
            ||
            (x1 == x2)
        )
        &&
        (
            (y1 < y2 && y1 + h1 > y2)
            ||
            (y2 < y1 && y2 + h2 > y1)
            ||
            (y1 == y2)
        )
        ;
}
} //namespace hg
