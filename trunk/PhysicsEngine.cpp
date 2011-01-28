#include "PhysicsEngine.h"

#include "TimeObjectListList.h"
#include "ObjectList.h"
#include "TimeDirection.h"
#include "ConcurrentTimeMap.h"
#include "ConcurrentTimeSet.h"

#include <iostream>
#include <map>
#include <vector>

#include <cassert>
#include <cmath>

using namespace ::std;
namespace hg {

PhysicsEngine::PhysicsEngine(const ::boost::multi_array<bool, 2>& nwallmap,
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

TimeObjectListList PhysicsEngine::executeFrame(const ObjectList& arrivals,
                                               const FrameID time,
                                               const std::vector<InputList>& playerInput,
                                               ConcurrentTimeMap& currentPlayerFramesAndDirections,
                                               ConcurrentTimeSet& nextPlayerFrames,
                                               ConcurrentTimeSet& winFrames) const
{
    std::vector<BoxInfo> nextBox;
    std::vector<GuyInfo> nextGuy;

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

    if (time.parentFrame() == FrameID())
    {
        // button state update
        buttonChecks(arrivals.getPlatformListRef(), arrivals.getBoxListRef(), arrivals.getGuyListRef(), arrivals.getButtonListRef(), nextButtonState, time);

        // Trigger system execution
        triggerSystem.getPlatformDestinations(nextButtonState, platformDesinations);
    }

	// platforms set their new location and velocity from trigger system data (and ofc their physical data)
	platformStep(arrivals.getPlatformListRef(), nextPlatform, platformDesinations, time);

	// pickup position update from platform

	// boxes do their crazy wizz-bang collision algorithm
    crappyBoxCollisionAlogorithm(arrivals.getBoxListRef(), nextBox, nextPlatform, time);

	// item simple collision algorithm

    // portal position update
    portalPositionUpdate(nextPlatform, arrivals.getPortalListRef(), nextPortal, time);

	// guys simple collision algorithm
	guyStep(arrivals.getGuyListRef(), time, playerInput,
            nextGuy, nextBox, nextPlatform, nextPortal, newDepartures, currentPlayerFramesAndDirections, nextPlayerFrames, pauseTimes);

    // button position update
    buttonPositionUpdate(nextPlatform, nextButtonState, arrivals.getButtonListRef(), nextButton, time);

    buildDepartures(nextBox, nextPlatform, nextPortal, nextButton, nextGuy,
                    arrivals.getBoxThiefListRef(), arrivals.getBoxExtraListRef(), arrivals.getGuyExtraListRef(),
                    newDepartures, time, pauseTimes);

    // compile all departures
    TimeObjectListList returnDepartures;

    for (NewDeparturesT::iterator it(newDepartures.begin()), end(newDepartures.end()); it != end; ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        returnDepartures.insertObjectList(it->first, ObjectList(it->second));
    }
    
    //Guys cannot win (ATM)
    winFrames.remove(time);
    
	// add data to departures
	return returnDepartures;
}

template <class Type, class TypeInfo> void PhysicsEngine::BuildDepartureForComplexEntities(
                                    const ::std::vector<TypeInfo>& next,
                                    const ::std::vector<RemoteDepartureEdit<Type> >& thief,
                                    const ::std::vector<RemoteDepartureEdit<Type> >& extra,
                                    NewDeparturesT& newDepartures,
                                    const FrameID time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const
{
    // builds departures for something that can move in complex ways throughout pause frames
	// adding, removal etc... things like guys and boxes
	for (size_t i = 0; i < next.size(); ++i)
	{
	    Type thing = next[i].info;

	    // the index of the next normal departure for a thing
		FrameID nextTime(time.nextFrame(thing.getTimeDirection()));

        if (next[i].time != nextTime)
        {
            newDepartures[next[i].time].add(thing);
            goto buildNext;
        }

        // check if the departure is to be stolen
		for (size_t t = 0; t < thief.size(); ++t)
		{
		    if (thief[t].getDeparture() == thing)
		    {
		        // by now the departure is known to be stolen
		        // if the departure is a pause departure the departure a level up must also be stolen
                if (thing.getPauseLevel() != 0)
                {
                    newDepartures[time.parentFrame()].addThief
                    (
                        RemoteDepartureEdit<Type>
                        (
                            time.universe().initiatorID(),
                            Type(thing, thing.getTimeDirection(), thing.getPauseLevel()-1),
                            true
                        )
                    );
                }

                // adds pause time departures to pause times before this one
                // also adds pause time departure to the pause time that stole the departue
                for (size_t j = 0; j < pauseTimes.size(); ++j)
                {
                    newDepartures[time.entryChildFrame(pauseTimes[j], thing.getTimeDirection())].add
                    (
                        Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
                    );

                    if (pauseTimes[j] == thief[t].getOrigin())
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
        if (time.nextFramePauseLevelDifference(thing.getTimeDirection()) != 0 && thing.getPauseLevel() != 0)
        {

        }
		else if (nextTime.isValidFrame())
		{
		    // simply depart to next frame
			newDepartures[nextTime].add(thing);
		}

        // add pause time departure to all spawned pause times
		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    newDepartures[time.entryChildFrame(pauseTimes[j], thing.getTimeDirection())].add
            (
                Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
            );
		}
        buildNext:
        ;
	}

	// special departures for things, from pause time
	// these things are pause time things that have changed location in pause time
	for (size_t i = 0; i < extra.size(); ++i)
	{
	    Type thing = extra[i].getDeparture();

	    // the index of the next normal departure for a thing
		FrameID nextTime(time.nextFrame(thing.getTimeDirection()));

        // check if the departure is to be stolen
		for (size_t t = 0; t < thief.size(); ++t)
		{
		    if (thief[t].getDeparture() == thing)
		    {
		        // by now the departure is known to be stolen
		        // if the departure is a pause departure the departure a level up must also be stolen
                if (thing.getPauseLevel() != 0)
                {
                    newDepartures[time.parentFrame()].addThief
                    (
                        RemoteDepartureEdit<Type>
                        (
                            time.universe().initiatorID(),
                            Type(thing, thing.getTimeDirection(), thing.getPauseLevel()-1),
                            true
                        )
                    );
                }

                // adds pause time departures to pause times before this one
                // also adds pause time departure to the pause time that stole the departue
                // CHANGE FROM NORMAL THING HANDLING: only add to pause times that occur after the extra thing pause time
                for (size_t j = 0; j < pauseTimes.size(); ++j)
                {
                    if (extra[i].getOrigin() < pauseTimes[j])
                    {
                        newDepartures[time.entryChildFrame(pauseTimes[j], thing.getTimeDirection())].add
                        (
                            Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
                        );
                    }

                    if (thief[t].getOrigin() == pauseTimes[j])
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
        if (thing.getPauseLevel() != 0 && time.nextFramePauseLevelDifference(thing.getTimeDirection()) != 0)
        {

        }
		else if (nextTime.isValidFrame() && extra[i].getPropIntoNormal())
		{
		    // simply depart to next frame
			newDepartures[nextTime].add(thing);
		}

        // CHANGE FROM NORMAL THING HANDLING: add pause time departure to pause times after the current one
		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    if (extra[i].getOrigin() < pauseTimes[j])
            {
                newDepartures[time.entryChildFrame(pauseTimes[j], thing.getTimeDirection())].add
                (
                    Type(thing, thing.getTimeDirection(), thing.getPauseLevel()+1)
                );
            }
		}
        buildNextExtra:
        ;
	}

}

template <class Type> void PhysicsEngine::BuildDepartureForReallySimpleThing(
                                    const ::std::vector<Type>& next,
                                    NewDeparturesT& newDepartures,
                                    const FrameID time,
                                    std::vector<PauseInitiatorID>& pauseTimes
                                    ) const
{
    for (size_t i = 0; i < next.size(); ++i)
	{
		FrameID nextTime(time.nextFrame(next[i].getTimeDirection()));

		if (nextTime.isValidFrame() && (next[i].getPauseLevel() == 0 || time.nextFramePauseLevelDifference(next[i].getTimeDirection()) == 0))
		{
			newDepartures[nextTime].add(next[i]);
		}

		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    newDepartures[time.entryChildFrame(pauseTimes[j], next[i].getTimeDirection())].add
            (
                Type(next[i], next[i].getTimeDirection(), next[i].getPauseLevel()+1)
            );
		}
	}
}


void PhysicsEngine::buildDepartures(const ::std::vector<BoxInfo>& nextBox,
                        const ::std::vector<Platform>& nextPlatform,
                        const ::std::vector<Portal>& nextPortal,
                        const ::std::vector<Button>& nextButton,
                        const ::std::vector<GuyInfo>& nextGuy,
                        const ::std::vector<RemoteDepartureEdit<Box> >& boxThief,
                        const ::std::vector<RemoteDepartureEdit<Box> >& boxExtra,
                        const ::std::vector<RemoteDepartureEdit<Guy> >& guyExtra,
                        NewDeparturesT& newDepartures,
                        const FrameID time,
                        std::vector<PauseInitiatorID>& pauseTimes

                                    /*const vector<BoxInfo>& nextBox,
                                    const vector<Platform>& nextPlatform,
                                    const vector<Portal>& nextPortal,
                                    const vector<Button>& nextButton,
                                    const vector<GuyInfo>& nextGuy,
                                    const vector<RemoteDepartureEdit<Box> >& boxThief,
                                    const vector<RemoteDepartureEdit<Box> >& boxExtra,
                                    const vector<RemoteDepartureEdit<Guy> >& guyExtra,
                                    map<FrameID, MutableObjectList>& newDepartures,
                                    const FrameID time,
                                    std::vector<PauseInitiatorID>& pauseTimes*/
                                    ) const
{

    // pause times initiated in the frame must be sorted
    sort(pauseTimes.begin(), pauseTimes.end());

	BuildDepartureForComplexEntities<Box,BoxInfo>(nextBox, boxThief, boxExtra, newDepartures, time, pauseTimes);

    // build departures for guys
	for (size_t i = 0; i < nextGuy.size(); ++i)
	{
	    Guy guyData = nextGuy[i].info;

		FrameID nextTime(time.nextFrame(guyData.getTimeDirection()));

        // Depart to next frame but do not depart if the guy is paused and it is the end of a pause time
        if (nextGuy[i].time == nextTime)
        {
            if (nextTime.isValidFrame() && (guyData.getPauseLevel() == 0 || time.nextFramePauseLevelDifference(guyData.getTimeDirection()) == 0))
            {
                newDepartures[nextTime].add(guyData);
            }
        }
        else
        {
            newDepartures[nextGuy[i].time].add(guyData);
        }

        // Add extra departures if departing from pause time so that subsequent pause times in the same parent frame see
        // this guy's state at the end of it's pause time. Pause times are ordered.
        if (time.nextFramePauseLevelDifference(guyData.getTimeDirection()) != 0 and guyData.getPauseLevel() == 0 )
        {
            int universes = time.nextFramePauseLevelDifference(guyData.getTimeDirection());
            FrameID parTime = time;
            do
            {
                PauseInitiatorID parInit = parTime.universe().initiatorID();
                parTime = parTime.parentFrame();
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
                            -1, guyData.getSubimage()
                        ),
                        false
                    )

                );
                universes--;
            }
            while (universes > 0);
        }


        if (nextGuy[i].time.parentFrame() == time)
        {
            // if the guy is departing to paused don't add it to pause times after this one
            PauseInitiatorID pauseID = nextGuy[i].time.universe().initiatorID();
            for (size_t j = 0; j < pauseTimes.size(); ++j)
            {
                if (pauseID == pauseTimes[j])
                {
                    break;
                }
                newDepartures[time.entryChildFrame(pauseTimes[j], guyData.getTimeDirection())].add
                (
                    Guy(guyData.getX(), guyData.getY(), guyData.getXspeed(), guyData.getYspeed(),
                        guyData.getWidth(),guyData.getHeight(), guyData.getRelativeToPortal(), guyData.getSupported(),
                        guyData.getBoxCarrying(), guyData.getBoxCarrySize(),
                        guyData.getBoxCarryDirection(), guyData.getBoxPauseLevel(),
                        guyData.getTimeDirection(), guyData.getPauseLevel()+1,
                        -1, guyData.getSubimage()
                    )
                );
            }
        }
        else
        {
            // add pause guy to every pause time universe from this frame
            for (size_t j = 0; j < pauseTimes.size(); ++j)
            {
                newDepartures[time.entryChildFrame(pauseTimes[j], guyData.getTimeDirection())].add
                (
                    Guy(guyData.getX(), guyData.getY(), guyData.getXspeed(), guyData.getYspeed(),
                        guyData.getWidth(),guyData.getHeight(), guyData.getRelativeToPortal(), guyData.getSupported(),
                        guyData.getBoxCarrying(), guyData.getBoxCarrySize(),
                        guyData.getBoxCarryDirection(), guyData.getBoxPauseLevel(),
                        guyData.getTimeDirection(), guyData.getPauseLevel()+1,
                        -1, guyData.getSubimage()
                    )
                );
            }
        }
	}

	// build departure for extra guys (purely graphical things to do with pause order)
	for (size_t i = 0; i < guyExtra.size(); ++i)
	{
        // add pause time departure to pause times after the current one
		for (size_t j = 0; j < pauseTimes.size(); ++j)
		{
		    if (guyExtra[i].getOrigin() < pauseTimes[j])
            {
                newDepartures[time.entryChildFrame(pauseTimes[j], guyExtra[i].getDeparture().getTimeDirection())].add
                (
                    Guy(guyExtra[i].getDeparture())
                );
            }
		}
	}

	// simple things
    BuildDepartureForReallySimpleThing<Platform>(nextPlatform, newDepartures, time, pauseTimes);
    BuildDepartureForReallySimpleThing<Button>(nextButton, newDepartures, time, pauseTimes);
    BuildDepartureForReallySimpleThing<Portal>(nextPortal, newDepartures, time, pauseTimes);
}

void PhysicsEngine::guyStep(const ::std::vector<Guy>& oldGuyList,
                            FrameID time,
                            const ::std::vector<InputList>& playerInput,
                            std::vector<GuyInfo>& nextGuy,
                            ::std::vector<BoxInfo>& nextBox,
                            const ::std::vector<Platform>& nextPlatform,
                            const ::std::vector<Portal>& nextPortal,
                            NewDeparturesT& newDepartures,
                            ConcurrentTimeMap& currentPlayerFramesAndDirections,
                            ConcurrentTimeSet& nextPlayerFrames,
                            std::vector<PauseInitiatorID>& pauseTimes
                            ) const
{
	vector<int> x;
	vector<int> y;
	vector<int> xspeed;
	vector<int> yspeed;
	vector<char> supported;

    x.reserve(oldGuyList.size());
    y.reserve(oldGuyList.size());
    xspeed.reserve(oldGuyList.size());
    yspeed.reserve(oldGuyList.size());
    supported.reserve(oldGuyList.size());

	// position, velocity, collisions
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
	    if (oldGuyList[i].getRelativeToPortal() == -1)
	    {
            x.push_back(oldGuyList[i].getX());
            y.push_back(oldGuyList[i].getY());
	    }
	    else
	    {
	        Portal relativePortal(nextPortal[oldGuyList[i].getRelativeToPortal()]);
            x.push_back(relativePortal.getX() + oldGuyList[i].getX());
            y.push_back(relativePortal.getY() + oldGuyList[i].getY());
	    }
        xspeed.push_back(0);
        yspeed.push_back(oldGuyList[i].getYspeed() + gravity);
        supported.push_back(false);

        if (oldGuyList[i].getRelativeIndex() < playerInput.size() && oldGuyList[i].getPauseLevel() == 0)
        {
            size_t relativeIndex = oldGuyList[i].getRelativeIndex();
            const InputList& input = playerInput[relativeIndex];

            int width = oldGuyList[i].getWidth();
            int height = oldGuyList[i].getHeight();

             // jump
            if (oldGuyList[i].getSupported() && input.getUp())
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
            for (unsigned int j = 0; j < nextPlatform.size(); ++j)
            {
                int pX = nextPlatform[j].getX();
                int pY = nextPlatform[j].getY();
                TimeDirection pDirection = nextPlatform[j].getTimeDirection();
                if (pDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= nextPlatform[j].getXspeed();
                    pY -= nextPlatform[j].getYspeed();
                }
                int pWidth = nextPlatform[j].getWidth();
                int pHeight = nextPlatform[j].getHeight();

                if (IntersectingRectangles(x[i], newY, width, height, pX, pY, pWidth, pHeight, false))
                {
                    if (newY+height/2 < pY+pHeight/2)
                    {
                        newY = pY-height;
                        xspeed[i] = pDirection*oldGuyList[i].getTimeDirection()*nextPlatform[j].getXspeed();
                        supported[i] = true;
                    }
                    else
                    {
                        newY = pY+pHeight;
                    }
                }
            }

            // box collision
            for (unsigned int j = 0; j < nextBox.size(); ++j)
            {
                int boxX = nextBox[j].info.getX();
                int boxY = nextBox[j].info.getY();
                int boxXspeed = nextBox[j].info.getXspeed();
                int boxYspeed = nextBox[j].info.getYspeed();
                int boxSize = nextBox[j].info.getSize();
                TimeDirection boxDirection = nextBox[j].info.getTimeDirection();
                if (x[i] <= boxX+boxSize && x[i]+width >= boxX)
                {
                    if (nextBox[j].info.getPauseLevel() > 0)
                    {
                        if (newY+height >= boxY && newY+height-yspeed[i] <= boxY)
                        {
                            newY = boxY-height;
                            xspeed[i] = 0;
                            supported[i] = true;
                        }
                    }
                    else if (boxDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
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
            int newX = x[i] + xspeed[i];

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
            for (unsigned int j = 0; j < nextPlatform.size(); ++j)
            {
                int pX = nextPlatform[j].getX();
                int pY = nextPlatform[j].getY();
                TimeDirection pDirection = nextPlatform[j].getTimeDirection();
                if (pDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= nextPlatform[j].getXspeed();
                    pY -= nextPlatform[j].getYspeed();
                }
                int pWidth = nextPlatform[j].getWidth();
                int pHeight = nextPlatform[j].getHeight();

                if (IntersectingRectangles(newX, newY, width, height, pX, pY, pWidth, pHeight, false))
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

	vector<char> carry;
	vector<int> carrySize;
	vector<TimeDirection> carryDirection;
	vector<int> carryPauseLevel;
    carry.reserve(oldGuyList.size());
    carrySize.reserve(oldGuyList.size());
    carryDirection.reserve(oldGuyList.size());
    carryPauseLevel.reserve(oldGuyList.size());
	// box carrying
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{

		carry.push_back(oldGuyList[i].getBoxCarrying());
		carrySize.push_back(0);
		carryDirection.push_back(hg::INVALID);
		carryPauseLevel.push_back(0);

        if (oldGuyList[i].getRelativeIndex() < playerInput.size() && oldGuyList[i].getPauseLevel() == 0)
        {

            size_t relativeIndex = oldGuyList[i].getRelativeIndex();
            const InputList& input = playerInput[relativeIndex];

            if (carry[i])
            {
                bool droppable = false;
                if (input.getDown())
                {
                    int width = oldGuyList[i].getWidth();
                    int dropX = x[i] + width/2 - oldGuyList[i].getBoxCarrySize()/2;
                    int dropY = y[i] - oldGuyList[i].getBoxCarrySize();
                    int dropSize = oldGuyList[i].getBoxCarrySize();

                    if (!wallAt(dropX, dropY, dropSize, dropSize))
                    {
                        droppable = true;
                        for (unsigned int j = 0; droppable && j < nextBox.size(); ++j)
                        {
                            if (IntersectingRectangles(nextBox[j].info.getX(), nextBox[j].info.getY(), nextBox[j].info.getSize(), nextBox[j].info.getSize(),
                                                      dropX, dropY, dropSize, dropSize, false))
                            {
                                droppable = false;
                            }
                        }
                        for (unsigned int j = 0; droppable && j < nextPlatform.size(); ++j)
                        {
                            if (IntersectingRectangles(nextPlatform[j].getX(), nextPlatform[j].getY(),
                                                       nextPlatform[j].getWidth(), nextPlatform[j].getHeight(),
                                                      dropX, dropY, dropSize, dropSize, false))
                            {
                                droppable = false;
                                break;
                            }
                        }
                        if (droppable)
                        {
                            nextBox.push_back
                            (
                                PhysicsEngine::BoxInfo
                                (
                                    Box
                                    (
                                        dropX,
                                        dropY,
                                        0,
                                        0,
                                        dropSize,
                                        oldGuyList[i].getBoxCarryDirection(),
                                        oldGuyList[i].getBoxPauseLevel()
                                    ),
								time.nextFrame(oldGuyList[i].getBoxCarryDirection()),
                                false
                                )
                            );

                            if (oldGuyList[i].getBoxPauseLevel() != 0)
                            {
                                int pauseLevel = oldGuyList[i].getBoxPauseLevel();
                                FrameID parTime = time.parentFrame();
                                PauseInitiatorID parInit = time.universe().initiatorID();
                                int pauseLevelChange = 1;
                                while (pauseLevel > 0)
                                {
                                    newDepartures[parTime].addExtra
                                    (
                                        RemoteDepartureEdit<Box>
                                        (
                                            parInit,
                                            Box
                                            (
                                                dropX, dropY, 0, 0,
                                                dropSize, oldGuyList[i].getBoxCarryDirection(),
                                                oldGuyList[i].getBoxPauseLevel()-pauseLevelChange
                                            ),
                                            true
                                        )

                                    );
                                    --pauseLevel;
                                    ++pauseLevelChange;
                                    parInit = parTime.universe().initiatorID();
                                    parTime = parTime.parentFrame();
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
                    carrySize[i] = oldGuyList[i].getBoxCarrySize();
                    carryDirection[i] = oldGuyList[i].getBoxCarryDirection();
                    carryPauseLevel[i] = oldGuyList[i].getBoxPauseLevel();
                }
            }
            else
            {
                if (input.getDown())
                {
                    int width = oldGuyList[i].getWidth();
                    int height = oldGuyList[i].getHeight();

                    for (unsigned int j = 0; j < nextBox.size(); ++j)
                    {
                        int boxX = nextBox[j].info.getX();
                        int boxY = nextBox[j].info.getY();
                        int boxSize = nextBox[j].info.getSize();
                        if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY))
                        {
                            carry[i] = true;
                            carrySize[i] = boxSize;
                            carryDirection[i] = nextBox[j].info.getTimeDirection();
                            carryPauseLevel[i] = nextBox[j].info.getPauseLevel();
                            if (nextBox[j].info.getPauseLevel() != 0)
                            {
                                newDepartures[time.parentFrame()].addThief
                                (
                                    RemoteDepartureEdit<Box>
                                    (
                                        time.universe().initiatorID(),
                                        Box
                                        (
                                            boxX, boxY, nextBox[j].info.getXspeed(), nextBox[j].info.getYspeed(),
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
    bool currentPlayerInFrame(false);
    TimeDirection currentPlayerDirection(INVALID);
    bool nextPlayerInFrame(false);
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getPauseLevel() != 0)
	    {
            nextGuy.push_back
            (
                PhysicsEngine::GuyInfo
                (
                    oldGuyList[i],time.nextFrame(oldGuyList[i].getTimeDirection())
                )
            );
	    }
        else if (oldGuyList[i].getRelativeIndex() < playerInput.size())
        {
            // animation
            int nextSubimage = oldGuyList[i].getSubimage() + 1;
            if (nextSubimage > Guy::animationLength)
            {
                nextSubimage = 0;
            }

            size_t relativeIndex(oldGuyList[i].getRelativeIndex());
            const InputList& input = playerInput[relativeIndex];
            int nextCarryPauseLevel = carryPauseLevel[i];
            int relativeToPortal = -1;

            // add departure for guy at the appropriate time
            TimeDirection nextTimeDirection = oldGuyList[i].getTimeDirection();
            FrameID nextTime(time.nextFrame(nextTimeDirection));
            assert(time.isValidFrame());

            bool normalDeparture = true;

            if (input.getAbility() == hg::TIME_JUMP)
            {
                nextTime = input.getFrameIdParam(0);
                normalDeparture = false;
            }
            else if (input.getAbility() == hg::TIME_REVERSE)
            {
                normalDeparture = false;
                nextTimeDirection *= -1;
                nextTime = time.nextFrame(nextTimeDirection);
                carryDirection[i] *= -1;

                if (time.nextFramePauseLevelDifference(nextTimeDirection) != 0)
                {
                    nextCarryPauseLevel -= time.nextFramePauseLevelDifference(nextTimeDirection);
                    if (nextCarryPauseLevel < 0)
                    {
                        nextCarryPauseLevel = 0;
                    }
                }
            }
            else if (input.getAbility() == hg::PAUSE_TIME and time.frame() == 3000) // FOR TESTING, REMOVE
            {
                normalDeparture = false;
                PauseInitiatorID pauseID = PauseInitiatorID(pauseinitiatortype::GUY, relativeIndex, 500);
                nextTime = time.entryChildFrame(pauseID, oldGuyList[i].getTimeDirection());

                pauseTimes.push_back(pauseID);
            }
            else if (input.getUse() == true)
            {
                int mx = x[i] + oldGuyList[i].getWidth();
                int my = y[i] + oldGuyList[i].getHeight();
                for (unsigned int j = 0; j < nextPortal.size(); ++j)
                {
                    int px = nextPortal[j].getX();
                    int py = nextPortal[j].getY();
                    int pw = nextPortal[j].getWidth();
                    int ph = nextPortal[j].getHeight();
                    if (PointInRectangle(mx, my, px, py, pw, ph, true) && nextPortal[j].getPauseLevel() == 0 &&
                        nextPortal[j].getActive() && nextPortal[j].getCharges() != 0) // charges not fully implemented
                    {
                        FrameID portalTime;
                        if (nextPortal[j].getRelativeTime() == true)
                        {
                            portalTime = FrameID(time.frame() + nextPortal[j].getTimeDestination(), 10800);
                        }
                        else
                        {
                            portalTime = FrameID(nextPortal[j].getTimeDestination(), 10800);
                        }
                        if (portalTime.isValidFrame())
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
                if (time.nextFramePauseLevelDifference(nextTimeDirection) != 0)
                {
                    nextCarryPauseLevel -= time.nextFramePauseLevelDifference(nextTimeDirection);
                    if (nextCarryPauseLevel < 0)
                    {
                        nextCarryPauseLevel = 0;
                    }
                }
            }

            if (playerInput.size() - 1 == relativeIndex)
            {
                currentPlayerDirection = oldGuyList[i].getTimeDirection();
                currentPlayerInFrame = true;
                //cout << "nextPlayerFrame set to: " << nextPlayerFrame.frame() << "  " << x[i] << "\n";
            }

            if (nextTime.isValidFrame())
            {
                nextGuy.push_back
                (
                    GuyInfo(Guy(x[i], y[i], xspeed[i], yspeed[i],
                        oldGuyList[i].getWidth(), oldGuyList[i].getHeight(),
                        relativeToPortal, supported[i],
                        carry[i], carrySize[i], carryDirection[i], nextCarryPauseLevel,
                        nextTimeDirection, 0,
                        relativeIndex+1, nextSubimage),
                    nextTime)
                );
            }
        }
        else
        {
            assert(oldGuyList[i].getRelativeIndex() == playerInput.size());
            nextPlayerInFrame = true;
        }
    }
    if (currentPlayerInFrame) {
        currentPlayerFramesAndDirections.add(time, currentPlayerDirection);
    }
    else {
        currentPlayerFramesAndDirections.remove(time);
    }
    if (nextPlayerInFrame) {
        nextPlayerFrames.add(time);
    }
    else {
        nextPlayerFrames.remove(time);
    }    
}

void PhysicsEngine::crappyBoxCollisionAlogorithm(const ::std::vector<Box>& oldBoxList,
                                                ::std::vector<BoxInfo>& nextBox,
                                                std::vector<Platform>& nextPlatform,
                                                const FrameID time) const
{
	for (vector<Box>::const_iterator i(oldBoxList.begin()), iend(oldBoxList.end()); i != iend; ++i)
	{

	    if (i->getPauseLevel() != 0)
	    {
	        nextBox.push_back
            (
                PhysicsEngine::BoxInfo
                (
                    *i,
                    time.nextFrame(i->getTimeDirection()),
                    true
                )
            );

	        continue;
	    }

		int x = i->getX();
		int y = i->getY();
		int xspeed = i->getXspeed();
		int yspeed = i->getYspeed() + gravity;
		int size = i->getSize();

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
                if (PhysicsEngine::IntersectingRectangles(x, y, size, size, boxX, boxY, boxSize, boxSize, false))
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
            if (PhysicsEngine::IntersectingRectangles(x, y, size, size, j->getX(), j->getY(), j->getHeight(), j->getWidth(), false))
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
        for (vector<Box>::const_iterator j(oldBoxList.begin()), jend(oldBoxList.end()); j != jend; ++j)
        {
            if (j != i)
            {
                int boxX = j->getX();
                int boxY = j->getY();
                int boxSize = j->getSize();
                if (PhysicsEngine::IntersectingRectangles(x, newY, size, size, boxX, boxY, boxSize, boxSize, true))
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
        for (unsigned int j = 0; j < nextPlatform.size(); ++j)
        {
            int pX = nextPlatform[j].getX();
            int pY = nextPlatform[j].getY();
            TimeDirection pDirection = nextPlatform[j].getTimeDirection();
            if (pDirection*i->getPauseLevel() == 0 && pDirection*i->getTimeDirection() == hg::REVERSE)
            {
                pX -= nextPlatform[j].getXspeed();
                pY -= nextPlatform[j].getYspeed();
            }
            int pWidth = nextPlatform[j].getWidth();
            int pHeight = nextPlatform[j].getHeight();

            if (IntersectingRectangles(x, newY, size, size, pX, pY, pWidth, pHeight, false))
            {
                if (newY+size/2 < pY+pHeight/2)
                {
                    newY = pY-size;
                    xspeed = pDirection*i->getTimeDirection()*nextPlatform[j].getXspeed();
                    supported = true;
                }
                else
                {
                    newY = pY+pHeight;
                }
            }
        }

        // ** X component **
        int newX = x + xspeed;

        for (vector<Box>::const_reverse_iterator j(oldBoxList.rbegin()), jend(oldBoxList.rend()); j != jend; ++j)
        {
            if ((j.base() - 1) != i )
            {
                {
                    int boxX = j->getX();
                    int boxY = j->getY();
                    int boxSize = j->getSize();
                    if (PhysicsEngine::IntersectingRectangles(newX, newY, size, size, boxX, boxY, boxSize, boxSize, true))
                    {
                        if (newX + size/2 < boxX + boxSize/2)
                        {
                            newX = x + j->getXspeed()/10;//boxX - size;
                        }
                        else
                        {
                            newX = x + j->getXspeed()/10;//boxX + boxSize;
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
            if (pDirection*i->getTimeDirection() == hg::REVERSE)
            {
                pX -= nextPlatform[j].getXspeed();
                pY -= nextPlatform[j].getYspeed();
            }
            int pWidth = nextPlatform[j].getWidth();
            int pHeight = nextPlatform[j].getHeight();

            if (IntersectingRectangles(newX, newY, size, size, pX, pY, pWidth, pHeight, false))
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
            PhysicsEngine::BoxInfo
            (
                Box
                (
                    x,
                    y,
                    xspeed,
                    yspeed,
                    size,
                    i->getTimeDirection(),
                    0
                ),
                time.nextFrame(i->getTimeDirection()),
                supported
            )
        );

	}
}

void PhysicsEngine::platformStep(const ::std::vector<Platform>& oldPlatformList,
                                  std::vector<Platform>& nextPlatform,
                                  const std::vector<PlatformDestination>& pd,
                                  const FrameID& time) const
{

    for (unsigned int i = 0; i < oldPlatformList.size(); ++i)
    {
        if (oldPlatformList[i].getPauseLevel() != 0)
	    {
	        nextPlatform.push_back(oldPlatformList[i]);
	        continue;
	    }

        int x = oldPlatformList[i].getX();
		int y = oldPlatformList[i].getY();
		int xspeed = oldPlatformList[i].getXspeed();
		int yspeed = oldPlatformList[i].getYspeed();

        // X component
        if (pd[i].getX() != x)
        {
            if (abs(pd[i].getX() - x) <= abs(xspeed) && abs(xspeed) <= pd[i].getXdeccel())
            {
                xspeed = pd[i].getX() - x;
            }
            else
            {
                int direction = abs(x - pd[i].getX())/(x - pd[i].getX());

                if (xspeed*direction >= 0)
                {
                    xspeed -= direction*pd[i].getXdeccel();
                    if (xspeed*direction < 0)
                    {
                        xspeed = 0;
                    }
                }
                else
                {
                    // if the platform can still stop if it fully accelerates
                    if (abs(x - pd[i].getX()) > ((int)(pow((double)(xspeed - direction*pd[i].getXaccel()),2))*3/(2*pd[i].getXdeccel())))
                    {
                        // fully accelerate
                        xspeed -= direction*pd[i].getXaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(x - pd[i].getX()) > ((int)(pow((double)(xspeed),2))*3/(2*pd[i].getXdeccel())))
                    {
                        // set speed to required speed
                        xspeed = -direction*(int)(floor((double)(abs(x - pd[i].getX())*pd[i].getXdeccel()*2/3)));
                    }
                    else
                    {
                        xspeed += direction*pd[i].getXdeccel();
                    }
                }
            }
        }
        else
        {
            if (abs(xspeed) <= pd[i].getXdeccel())
            {
                xspeed = 0;
            }
            else
            {
                xspeed += abs(xspeed)/xspeed*pd[i].getXdeccel();
            }
        }

        if (abs(xspeed) > pd[i].getXspeed())
        {
            xspeed = abs(xspeed)/xspeed*pd[i].getXspeed();
        }

		x += xspeed;

        // Y component
        if (pd[i].getY() != y)
        {
            if (abs(pd[i].getY() - y) <= abs(yspeed) && abs(yspeed) <= pd[i].getYdeccel())
            {
                yspeed = pd[i].getY() - y;
            }
            else
            {
                int direction = abs(y - pd[i].getY())/(y - pd[i].getY());

                if (yspeed*direction > 0)
                {
                    yspeed -= direction*pd[i].getYdeccel();
                    if (yspeed*direction < 0)
                    {
                        yspeed = 0;
                    }
                }
                else
                {
                    // if the platform can still stop if it fully accelerates
                    if (abs(y - pd[i].getY()) > ((int)pow((double)yspeed - direction*pd[i].getYaccel(),2))*3/(2*pd[i].getYdeccel()))
                    {
                        // fully accelerate
                        yspeed -= direction*pd[i].getYaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(y - pd[i].getY()) > ((int)pow((double)yspeed,2))*3/(2*pd[i].getYdeccel()))
                    {
                        // set speed to required speed
                        yspeed = -direction*(int)floor(sqrt((double)abs(y - pd[i].getY())*pd[i].getYdeccel()*2/3));
                    }
                    else
                    {
                        yspeed += direction*pd[i].getYdeccel();
                    }
                }
            }
        }
        else
        {
            if (abs(yspeed) <= pd[i].getYdeccel())
            {
                yspeed = 0;
            }
            else
            {
                yspeed += abs(yspeed)/yspeed*pd[i].getYdeccel();
            }
        }

        if (abs(yspeed) > pd[i].getYspeed())
        {
            yspeed = abs(yspeed)/yspeed*pd[i].getYspeed();
        }

        y += yspeed;

		// send information
        nextPlatform.push_back(Platform(x, y, xspeed, yspeed, oldPlatformList[i].getWidth(), oldPlatformList[i].getHeight(), oldPlatformList[i].getIndex(), oldPlatformList[i].getTimeDirection(), 0));
    }
}

void PhysicsEngine::portalPositionUpdate(
        const std::vector<Platform>& nextPlatform,
        const ::std::vector<Portal>& oldPortalList,
        std::vector<Portal>& nextPortal,
        FrameID time
    ) const
{
    const ::std::vector< ::boost::tuple<int, int, int> >& attachments(attachmentMap.getPortalAttachmentRef());

    for (unsigned int i = 0; i < oldPortalList.size(); ++i)
	{
	    Portal data(oldPortalList[i]);

	    if (data.getPauseLevel() != 0)
	    {
	        nextPortal.push_back(data);
	        continue;
	    }

	    int x = data.getX();
	    int y = data.getY();

	    int index = data.getIndex();

	    if (attachments[index].get<0>() != -1)
	    {
	        int pid = attachments[index].get<0>();

	        if (nextPlatform[pid].getTimeDirection()*data.getTimeDirection() == hg::FORWARDS)
	        {
	            x = nextPlatform[pid].getX()+attachments[index].get<1>();
                y = nextPlatform[pid].getY()+attachments[index].get<2>();
	        }
	        else
	        {
	            x = nextPlatform[pid].getX()-nextPlatform[pid].getXspeed()+attachments[index].get<1>();
                y = nextPlatform[pid].getY()-nextPlatform[pid].getYspeed()+attachments[index].get<2>();
	        }
	    }

        nextPortal.push_back(Portal(x, y, x-data.getX(), y-data.getY(),  data.getWidth() , data.getHeight() , index, data.getTimeDirection(), data.getPauseLevel(),
                                    data.getCharges(), data.getActive(), data.getXdestination(), data.getYdestination(), data.getDestinationIndex(),
                                    data.getTimeDestination(), data.getRelativeTime() ));
	}

}

void PhysicsEngine::buttonPositionUpdate(
        const ::std::vector<Platform>& nextPlatform,
        const ::std::vector<char>& nextButtonState,
        const ::std::vector<Button>& oldButtonList,
        ::std::vector<Button>& nextButton,
        FrameID time
    ) const
{

    const ::std::vector< ::boost::tuple<int, int, int> >& attachments(attachmentMap.getButtonAttachmentRef());


    for (unsigned int i = 0; i < oldButtonList.size(); ++i)
	{
	    if (oldButtonList[i].getPauseLevel() != 0)
	    {
	        nextButton.push_back(oldButtonList[i]);
	        continue;
	    }

	    int x = oldButtonList[i].getX();
	    int y = oldButtonList[i].getY();

	    int index = oldButtonList[i].getIndex();

	    if (attachments[index].get<0>() != -1)
	    {
	        int pid = attachments[index].get<0>();

	        if (nextPlatform[pid].getTimeDirection()*oldButtonList[index].getTimeDirection() == hg::FORWARDS)
	        {
	            x = nextPlatform[pid].getX()+attachments[index].get<1>();
                y = nextPlatform[pid].getY()+attachments[index].get<2>();
	        }
	        else
	        {
	            x = nextPlatform[pid].getX()-nextPlatform[pid].getXspeed()+attachments[index].get<1>();
                y = nextPlatform[pid].getY()-nextPlatform[pid].getYspeed()+attachments[index].get<2>();
	        }
	    }

        nextButton.push_back(Button(x, y, x-oldButtonList[i].getX(), y-oldButtonList[i].getY(), index, nextButtonState[i], oldButtonList[i].getTimeDirection(), 0));
	}
}


void PhysicsEngine::buttonChecks(const ::std::vector<Platform>& oldPlatformList,
                                 const ::std::vector<Box>& oldBoxList,
                                const ::std::vector<Guy>& oldGuyList,
                                const ::std::vector<Button>& oldButtonList,
                                std::vector<char>& nextButton,
                                FrameID time) const
{

    const ::std::vector< ::boost::tuple<int, int, int> >& attachments(attachmentMap.getButtonAttachmentRef());

    for (unsigned int i = 0; i < oldButtonList.size(); ++i)
	{

	    int x = oldButtonList[i].getX();
	    int y = oldButtonList[i].getY();
	    int w = 3200;
	    int h = 800;
	    bool state = false;

	    int index = oldButtonList[i].getIndex();

	    if (attachments[index].get<0>() != -1)
	    {
	        int pid = attachments[index].get<0>();

	        if (oldPlatformList[pid].getTimeDirection()*oldButtonList[index].getTimeDirection() == hg::FORWARDS)
	        {
	            x = oldPlatformList[pid].getX()+attachments[index].get<1>();
                y = oldPlatformList[pid].getY()+attachments[index].get<2>();
	        }
	        else
	        {
	            x = oldPlatformList[pid].getX()-oldPlatformList[pid].getXspeed()+attachments[index].get<1>();
                y = oldPlatformList[pid].getY()-oldPlatformList[pid].getYspeed()+attachments[index].get<2>();
	        }
	    }

	    for (unsigned int j = 0; !state && j < oldBoxList.size(); ++j)
        {
            state = PhysicsEngine::IntersectingRectangles(x, y, w, h, oldBoxList[j].getX(), oldBoxList[j].getY(), oldBoxList[j].getSize(), oldBoxList[j].getSize(), true);
        }

        for (unsigned int j = 0; !state && j < oldGuyList.size(); ++j)
        {
            state = PhysicsEngine::IntersectingRectangles(x, y, w, h, oldGuyList[j].getX(), oldGuyList[j].getY(), oldGuyList[j].getWidth(), oldGuyList[j].getHeight(), true);
        }

        nextButton[oldButtonList[i].getIndex()] = state;
	}
}

bool PhysicsEngine::wallAt(int x, int y) const
{
	if (x < 0 || y < 0)
	{
		return true;
	}

	unsigned int aX = x/wallSize;
	unsigned int aY = y/wallSize;

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

bool PhysicsEngine::PointInRectangle(int px, int py, int x, int y, int w, int h, bool inclusive) const
{
     if (inclusive)
    {
        return
        (
            (px <= x + w && px >= x)
         &&
            (py <= y + h && py >= y)
        );
    }
    else
    {
        return
        (
            (px < x + w && px > x)
         &&
            (py < y + h && py > y)
        );
    }
}

bool PhysicsEngine::IntersectingRectangles(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool inclusive) const
{
    if (inclusive)
    {
        return
        (
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
        );
    }
    else
    {
        return
        (
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
        );
    }
}
} //namespace hg
