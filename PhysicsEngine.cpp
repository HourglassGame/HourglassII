#include "PhysicsEngine.h"

#include "Frame.h"

#include <boost/range/algorithm/for_each.hpp>
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
static const int SQUISHED_SPEED = 1600;
static const int JUMP_SPEED 	= -550;

PhysicsEngine::PhysicsEngine(
    const Environment& env,
    const TriggerSystem& triggerSystem) :
        env_(env),
        triggerSystem_(triggerSystem)
{
}
namespace {
    template<typename Object>
    struct ObjectAndTime
    {
        ObjectAndTime(const Object& nobject,
                Frame* nTime) :
                object(nobject),
                time(nTime)
        {}
        Object object;
        Frame* time;
    };
    struct SortObjectList
    {
        void operator()(std::pair<Frame* const, ObjectList>& toSortObjectListOf) const
        {
            toSortObjectListOf.second.sort();
        }
    };
    ObjectList calculatePausedStaticDepartures(const ObjectPtrList& arrivals) {
        ObjectList retv;
        foreach (const Platform& platform, arrivals.getPlatformListRef()) {
            retv.add(platform);
        }
        foreach (const Button& button, arrivals.getButtonListRef()) {
            retv.add(button);
        }
        foreach (const Portal& portal, arrivals.getPortalListRef()) {
            retv.add(portal);
        }
        return retv;
    }
    
    template<typename RandomAccessGuyRange>
    void guyStep(
        const Environment& env,
        const RandomAccessGuyRange& oldGuyList,
        Frame* time,
        const std::vector<InputList>& playerInput,
        std::vector<ObjectAndTime<Guy> >& nextGuy,
        std::vector<ObjectAndTime<Box> >& nextBox,
        const std::vector<Platform>& nextPlatform,
        const std::vector<Portal>& nextPortal,
        std::map<Frame*, ObjectList>& newDepartures,
        bool& currentPlayerFrame,
        bool& nextPlayerFrame,
        bool& winFrame,
        std::vector<PauseInitiatorID>& pauseTimes);
    
    template <
        typename RandomAccessBoxRange,
        typename RandomAccessPlatformRange>
    void boxCollisionAlogorithm(
        const Environment& env,
        const RandomAccessBoxRange& oldBoxList,
        std::vector<ObjectAndTime<Box> >& nextBox,
        RandomAccessPlatformRange& nextPlatform,
        Frame* time);
    
    bool explodeBoxesUpwards(
        std::vector<int>& x,
        const std::vector<int>& xTemp,
        std::vector<int>& y,
        const std::vector<int>& size,
        const std::vector<std::vector<int> >& links,
        bool firstTime,
        std::vector<char>& toBeSquished,
        const std::vector<int>& bound,
        std::size_t index,
        int boundSoFar);
    bool explodeBoxes(
        std::vector<int>& pos,
        const std::vector<int>& size,
        const std::vector<std::vector<int> >& links,
        std::vector<char>& toBeSquished,
        const std::vector<int>& bound,
        std::size_t index,
        int boundSoFar,
        int sign);
    void recursiveBoxCollision(
        std::vector<int>& majorAxis,
        const std::vector<int>& minorAxis,
        const std::vector<int>& size,
        const std::vector<char>& squished,
        std::vector<int>& boxesSoFar,
        std::size_t index);
        template <typename Type, typename RandomAccessEditRange>
    
    void buildDeparturesForComplexEntities(
        const std::vector<ObjectAndTime<Type> >& next,
        const RandomAccessEditRange& thieves,
        const RandomAccessEditRange& extras,
        std::map<Frame*, ObjectList>& newDepartures,
        Frame* time,
        std::vector<PauseInitiatorID>& pauseTimes);

    template <class Type>
    void buildDeparturesForReallySimpleThings(
        const std::vector<Type>& next,
        std::map<Frame*, ObjectList>& newDepartures,
        Frame* time,
        std::vector<PauseInitiatorID>& pauseTimes);
        
    template<
        typename RandomAccessBoxEditRangeA,
        typename RandomAccessBoxEditRangeB,
        typename RandomAccessGuyEditRange>
    void buildDepartures(
        const std::vector<ObjectAndTime<Box> >& nextBox,
        const std::vector<Platform>& nextPlatform,
        const std::vector<Portal>& nextPortal,
        const std::vector<Button>& nextButton,
        const std::vector<ObjectAndTime<Guy> >& nextGuy,
        const RandomAccessBoxEditRangeA& boxThieves,
        const RandomAccessBoxEditRangeB& extraBoxes,
        const RandomAccessGuyEditRange& extraGuys,
        std::map<Frame*, ObjectList>& newDepartures,
        Frame* time,
        std::vector<PauseInitiatorID>& pauseTimes);
        
    bool wallAtInclusive(const Environment& env, int x, int y, int w, int h);
    bool wallAtExclusive(const Environment& env, int x, int y, int w, int h);
        
    bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h);
    bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
    bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
}


PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
    const ObjectPtrList& arrivals,
    Frame* time,
    const std::vector<InputList>& playerInput) const
{
    /*Static?*/ObjectList staticDepartures(
        time->parentFrame() == 0 ? 
            triggerSystem_.calculateStaticDepartures(arrivals, playerInput, time):
            calculatePausedStaticDepartures(arrivals));

    std::vector<ObjectAndTime<Box> > nextBox;

    // boxes do their crazy wizz-bang collision algorithm
    boxCollisionAlogorithm(env_, arrivals.getBoxListRef(), nextBox, staticDepartures.getPlatformListRef(), time);

    bool currentPlayerFrame(false);
    bool nextPlayerFrame(false);
    bool winFrame(false);

    std::vector<ObjectAndTime<Guy> > nextGuy;
    
    std::map<Frame*, ObjectList> newDepartures;

    std::vector<PauseInitiatorID> pauseTimes;

    // guys simple collision algorithm
    guyStep(
        env_,
        arrivals.getGuyListRef(),
        time,
        playerInput,
        nextGuy,
        nextBox,
        staticDepartures.getPlatformListRef(),
        staticDepartures.getPortalListRef(),
        newDepartures,
        currentPlayerFrame,
        nextPlayerFrame,
        winFrame,
        pauseTimes);

    buildDepartures(
        nextBox,
        staticDepartures.getPlatformListRef(),
        staticDepartures.getPortalListRef(),
        staticDepartures.getButtonListRef(),
        nextGuy,
        arrivals.getBoxThiefListRef(),
        arrivals.getBoxExtraListRef(),
        arrivals.getGuyExtraListRef(),
        newDepartures,
        time,
        pauseTimes);

    //Sort all object lists before returning to other code. They must be sorted for comparisons to work correctly.
    boost::for_each(newDepartures,SortObjectList());
    // add data to departures
    return PhysicsReturnT(newDepartures, currentPlayerFrame, nextPlayerFrame, winFrame);
}

namespace {
template <typename Type, typename RandomAccessEditRange>
void buildDeparturesForComplexEntities(
    const std::vector<ObjectAndTime<Type> >& next,
    const RandomAccessEditRange& thieves,
    const RandomAccessEditRange& extras,
    std::map<Frame*, ObjectList>& newDepartures,
    Frame* time,
    std::vector<PauseInitiatorID>& pauseTimes)
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
        foreach (const RemoteDepartureEdit<Type>& thief, thieves)
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

        // if this is the end of the universe depart to null-frame as workaround for flicker
        if (time->nextFramePauseLevelDifference(thing.getTimeDirection()) != 0 && thing.getPauseLevel() != 0)
        {
            newDepartures[0].add(thing);
        }
        //otherwise depart normally
        else
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
    foreach (const RemoteDepartureEdit<Type>& extra, extras)
    {
        const Type& thing(extra.getDeparture());

        // the index of the next normal departure for a thing
        Frame* nextTime(time->nextFrame(thing.getTimeDirection()));

        // check if the departure is to be stolen
        foreach (const RemoteDepartureEdit<Type>& thief, thieves)
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
        else if (extra.getPropIntoNormal())
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
void buildDeparturesForReallySimpleThings(
    const std::vector<Type>& next,
    std::map<Frame*, ObjectList>& newDepartures,
    Frame* time,
    std::vector<PauseInitiatorID>& pauseTimes)
{
    foreach (const Type& thing, next)
    {
        Frame* nextTime(time->nextFrame(thing.getTimeDirection()));

        if (thing.getPauseLevel() == 0 || time->nextFramePauseLevelDifference(thing.getTimeDirection()) == 0)
        {
            newDepartures[nextTime].add(thing);
        }
        else {
            //workaround for flicker, object departs to null frame and so never arrives anywhere
            newDepartures[0].add(thing);
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

template<
    typename RandomAccessBoxEditRangeA,
    typename RandomAccessBoxEditRangeB,
    typename RandomAccessGuyEditRange>
void buildDepartures(
    const std::vector<ObjectAndTime<Box> >& nextBox,
    const std::vector<Platform>& nextPlatform,
    const std::vector<Portal>& nextPortal,
    const std::vector<Button>& nextButton,
    const std::vector<ObjectAndTime<Guy> >& nextGuy,
    const RandomAccessBoxEditRangeA& boxThieves,
    const RandomAccessBoxEditRangeB& extraBoxes,
    const RandomAccessGuyEditRange& extraGuys,
    std::map<Frame*, ObjectList>& newDepartures,
    Frame* time,
    std::vector<PauseInitiatorID>& pauseTimes)
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
            if (guyData.getPauseLevel() == 0 || time->nextFramePauseLevelDifference(guyData.getTimeDirection()) == 0)
            {
                newDepartures[nextTime].add(guyData);
            }
            else {
                //if it is end of pause time and guy is paused then add departure to null-frame as workaround for flicker
                newDepartures[0].add(guyData);
            }

        }
        else
        {
            newDepartures[guyAndTime.time].add(guyData);
        }

        // Add extra departures if departing from pause time so that subsequent pause times in the same parent frame see
        // this guy's state at the end of it's pause time. Pause times are ordered.
        if (nextTime && time->nextFramePauseLevelDifference(guyData.getTimeDirection()) != 0 and guyData.getPauseLevel() == 0 )
        {
            int universes(time->nextFramePauseLevelDifference(guyData.getTimeDirection()));
            Frame* parTime(time);
            do
            {
                const PauseInitiatorID& parInit(parTime->getInitiatorID());
                parTime = parTime->parentFrame();
                // This should be the ONLY place extra guys are added.
                // REMEMBER: PAUSE TIME GUNS DO NOT WORK
                newDepartures[parTime].addExtra(
                    RemoteDepartureEdit<Guy>(
                        parInit,
                        Guy(guyData, 1),
                        false));
                --universes;
            }
            while (universes > 0);
        }


        if (guyAndTime.time && guyAndTime.time->parentFrame() == time)
        {
            // if the guy is departing to paused don't add it to pause times after this one
            const PauseInitiatorID& pauseID(guyAndTime.time->getInitiatorID());
            foreach (const PauseInitiatorID& pauseTime, pauseTimes)
            {
                if (pauseID == pauseTime)
                {
                    break;
                }
                newDepartures[time->entryChildFrame(pauseTime, guyData.getTimeDirection())].add(Guy(guyData, 1));
            }
        }
        else
        {
            // add pause guy to every pause time universe from this frame
            foreach (const PauseInitiatorID& pauseTime, pauseTimes)
            {
                newDepartures[time->entryChildFrame(pauseTime, guyData.getTimeDirection())].add(Guy(guyData, 1));
            }
        }
    }

    // build departure for extra guys (purely graphical things to do with pause order)
    foreach (const RemoteDepartureEdit<Guy>& extraGuy, extraGuys)
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

template<typename RandomAccessGuyRange>
void guyStep(
    const Environment& env,
    const RandomAccessGuyRange& oldGuyList,
    Frame* time,
    const std::vector<InputList>& playerInput,
    std::vector<ObjectAndTime<Guy> >& nextGuy,
    std::vector<ObjectAndTime<Box> >& nextBox,
    const std::vector<Platform>& nextPlatform,
    const std::vector<Portal>& nextPortal,
    std::map<Frame*, ObjectList>& newDepartures,
    bool& currentPlayerFrame,
    bool& nextPlayerFrame,
    bool& winFrame,
    std::vector<PauseInitiatorID>& pauseTimes)
{
    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> xspeed;
    std::vector<int> yspeed;
    std::vector<char> supported;
    std::vector<int> supportedSpeed;
    std::vector<char> squished;
    std::vector<char> facing;

    x.reserve(boost::distance(oldGuyList));
    y.reserve(boost::distance(oldGuyList));
    xspeed.reserve(boost::distance(oldGuyList));
    yspeed.reserve(boost::distance(oldGuyList));
    supported.reserve(boost::distance(oldGuyList));
    supportedSpeed.reserve(boost::distance(oldGuyList));
    squished.reserve(boost::distance(oldGuyList));
    facing.reserve(boost::distance(oldGuyList));

    // position, velocity, collisions
    // check collisions in Y direction then do the same in X direction
    for (std::size_t i(0), isize(boost::distance(oldGuyList)); i < isize; ++i)
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
        yspeed.push_back(oldGuyList[i].getYspeed() + env.gravity);
        supported.push_back(false);
        squished.push_back(false);
        facing.push_back(oldGuyList[i].getFacing());

        if (oldGuyList[i].getIndex() < playerInput.size() && oldGuyList[i].getPauseLevel() == 0u)
        {
            std::size_t relativeIndex(oldGuyList[i].getIndex());
            const InputList& input = playerInput[relativeIndex];

            int width = oldGuyList[i].getWidth();
            int height = oldGuyList[i].getHeight();

            std::size_t boxThatIamStandingOn(std::numeric_limits<std::size_t>::max());

            // jump
            if (oldGuyList[i].getSupported() && input.getUp())
            {
                yspeed[i] = oldGuyList[i].getSupportedSpeed() + JUMP_SPEED;
            }

            // Y direction collisions
            int newY = y[i] + yspeed[i];

            // box collision (only occurs in Y direction
            for (std::size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
			{
				int boxX(nextBox[j].object.getX());
				int boxY(nextBox[j].object.getY());
				int boxXspeed(nextBox[j].object.getXspeed());
				int boxYspeed(nextBox[j].object.getYspeed());
				int boxSize(nextBox[j].object.getSize());
				TimeDirection boxDirection(nextBox[j].object.getTimeDirection());
				if (x[i] < boxX+boxSize && x[i]+width > boxX)
				{
					if (nextBox[j].object.getPauseLevel() > 0)
					{
						if (newY+height >= boxY && newY+height-yspeed[i] <= boxY)
						{
							boxThatIamStandingOn = j;
							newY = boxY-height;
							xspeed[i] = 0;
							supported[i] = true;
							supportedSpeed[i] = 0;
						}
					}
					else if (boxDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
					{
						if (newY+height >= boxY-boxYspeed && newY+height-yspeed[i] <= boxY)
						{
							boxThatIamStandingOn = j;
							newY = boxY-height-boxYspeed;
							xspeed[i] = -boxXspeed;
							supported[i] = true;
							supportedSpeed[i] = -boxYspeed;
						}

					}
					else
					{
						if (newY+height >= boxY && newY-yspeed[i]+height <= boxY-boxYspeed)
						{
							boxThatIamStandingOn = j;
							newY = boxY-height;
							xspeed[i] = boxXspeed;
							supported[i] = true;
							supportedSpeed[i] = boxYspeed;
						}
					}
				}
			}

            //check wall collision in Y direction
            if (yspeed[i] > 0) // down
            {
                if (env.wall.at(x[i], newY+height) || (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-width && env.wall.at(x[i]+width, newY+height)))
                {
                    newY = ((newY+height)/env.wall.segmentSize())*env.wall.segmentSize() - height;
                    supported[i] = true;
                    supportedSpeed[i] = 0;
                }
            }
            else if (yspeed[i] < 0) // up
            {
                if  (env.wall.at(x[i], newY) || (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-width && env.wall.at(x[i]+width, newY)))
                {
                    newY = (newY/env.wall.segmentSize() + 1)*env.wall.segmentSize();
                }
            }

            // check platform collision in Y direction
            foreach (const Platform& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection * oldGuyList[i].getTimeDirection() == hg::REVERSE && platform.getPauseLevel() == 0)
                {
                    pX -= platform.getXspeed();
                    pY -= platform.getYspeed();
                }
                int pWidth(platform.getWidth());
                int pHeight(platform.getHeight());

                if (IntersectingRectanglesExclusive(x[i], newY, width, height,
                		pX-pDirection * oldGuyList[i].getTimeDirection() * platform.getXspeed(), pY, pWidth, pHeight))
                {
                    if (newY+height/2 < pY+pHeight/2)
                    {
                        newY = pY-height;
                        xspeed[i] = pDirection * oldGuyList[i].getTimeDirection() * platform.getXspeed();
                        supported[i] = true;
                        if (platform.getPauseLevel() == 0)
                        {
                        	supportedSpeed[i] = pDirection * oldGuyList[i].getTimeDirection() * platform.getYspeed();
                        }
                        else
                        {
                        	supportedSpeed[i] = 0;
                        }
                    }
                    else
                    {
                        newY = pY + pHeight;
                    }
                }
            }

            // X direction stuff
            int newX(x[i] + xspeed[i]);

            //check wall collision in X direction
            if (input.getLeft())
            {
            	facing[i] = false;
                newX += -250;
            }
            else if (input.getRight())
            {
            	facing[i] = true;
                newX += 250;
            }

            if (newX-x[i] > 0) // right
            {
                if (env.wall.at(newX+width, newY) || (newY - (newY/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-height && env.wall.at(newX+width, newY+height)))
                {
                    newX = (newX+width)/env.wall.segmentSize()*env.wall.segmentSize() - width;
                }
            }
            else if (newX-x[i] < 0) // left
            {
                if (env.wall.at(newX, newY) || (newY - (newY/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-height && env.wall.at(newX, newY+height)))
                {
                    newX = (newX/env.wall.segmentSize() + 1)*env.wall.segmentSize();
                }
            }

            // platform collision
            foreach (const Platform& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE && platform.getPauseLevel() == 0)
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

            // Check inside a wall
			if (wallAtExclusive(env, newX, newY, width, height))
			{
				squished[i] = true;
			}

			// Check inside a platform
			foreach (const Platform& platform, nextPlatform)
			{
				int pX(platform.getX());
				int pY(platform.getY());
				TimeDirection pDirection(platform.getTimeDirection());
				if (pDirection * oldGuyList[i].getTimeDirection() == hg::REVERSE && platform.getPauseLevel() == 0)
				{
					pX -= platform.getXspeed();
					pY -= platform.getYspeed();
				}
				int pWidth(platform.getWidth());
				int pHeight(platform.getHeight());

				if (IntersectingRectanglesExclusive(newX, newY, width, height, pX, pY, pWidth, pHeight))
				{
					squished[i] = true;
				}
			}

			// Check inside the box that I am suppose to be on top of

			if (boxThatIamStandingOn != std::numeric_limits<std::size_t>::max())
			{
				int boxX(nextBox[boxThatIamStandingOn].object.getX());
				int boxY(nextBox[boxThatIamStandingOn].object.getY());
				int boxXspeed(nextBox[boxThatIamStandingOn].object.getXspeed());
				int boxYspeed(nextBox[boxThatIamStandingOn].object.getYspeed());
				int boxSize(nextBox[boxThatIamStandingOn].object.getSize());
				TimeDirection boxDirection(nextBox[boxThatIamStandingOn].object.getTimeDirection());

				if (boxDirection * oldGuyList[i].getTimeDirection() == hg::REVERSE)
				{
					boxX -= boxXspeed;
					boxY -= boxYspeed;
				}

				if (newX < boxX+boxSize && newX+width > boxX && newY < boxY+boxSize && newY+height > boxY)
				{
					squished[i] = true;
				}
        	}

			// If speed is too great I was squished (for example pushed to the side of a platform)
			if (newX-x[i] > SQUISHED_SPEED || newY-y[i] > SQUISHED_SPEED)
			{
				squished[i] = true;
			}

			// Apply Change
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
    for (std::size_t i(0), isize(boost::distance(oldGuyList)); i < isize; ++i)
    {

        carry.push_back(oldGuyList[i].getBoxCarrying());
        carrySize.push_back(0);
        carryDirection.push_back(hg::INVALID);
        carryPauseLevel.push_back(0);

        if (oldGuyList[i].getIndex() < playerInput.size() && oldGuyList[i].getPauseLevel() == 0 && not squished[i])
        {

            std::size_t relativeIndex(oldGuyList[i].getIndex());
            const InputList& input = playerInput[relativeIndex];

            if (carry[i])
            {
                bool droppable = false;
                if (input.getDown() && supported[i])
                {
                    int width(oldGuyList[i].getWidth());
                    int dropX(x[i] - oldGuyList[i].getBoxCarrySize());
                    int dropY(y[i]);
                    int dropSize(oldGuyList[i].getBoxCarrySize());

                    if (facing[i])
                    {
                    	dropX = x[i] + width;
                    }

                    if (!wallAtExclusive(env, dropX, dropY, dropSize, dropSize))
                    {
                        droppable = true;
                        for (std::size_t j(0), jsize(nextBox.size()); droppable && j < jsize; ++j)
                        {
                            if (IntersectingRectanglesExclusive(
                                    nextBox[j].object.getX(), nextBox[j].object.getY(),
                                    nextBox[j].object.getSize(), nextBox[j].object.getSize(),
                                    dropX, dropY, dropSize, dropSize))
                            {
                                droppable = false;
                            }
                        }
                        for (std::size_t j(0), jsize(nextPlatform.size()); droppable && j < jsize; ++j)
                        {
                            if (IntersectingRectanglesExclusive(
                                    nextPlatform[j].getX(), nextPlatform[j].getY(),
                                    nextPlatform[j].getWidth(), nextPlatform[j].getHeight(),
                                    dropX, dropY, dropSize, dropSize))
                            {
                                droppable = false;
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
                                        yspeed[i],
                                        dropSize,
                                        oldGuyList[i].getBoxCarryDirection(),
                                        oldGuyList[i].getBoxPauseLevel()
                                    ),
                                    time->nextFrame(oldGuyList[i].getBoxCarryDirection())//,
                                    //false
                                )
                            );

                            if (oldGuyList[i].getBoxPauseLevel() != 0)
                            {
                                int pauseLevel = oldGuyList[i].getBoxPauseLevel();
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
                                                dropX, dropY, 0, yspeed[i],
                                                dropSize, oldGuyList[i].getBoxCarryDirection(),
                                                oldGuyList[i].getBoxPauseLevel()-pauseLevelChange
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

                    for (size_t j(0), jsize(nextBox.size()); j < jsize; ++j)
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
                                newDepartures[time->parentFrame()].addThief(
                                    RemoteDepartureEdit<Box>(
                                        time->getInitiatorID(),
                                        Box(
                                            boxX, boxY, nextBox[j].object.getXspeed(), nextBox[j].object.getYspeed(),
                                            boxSize, carryDirection[i], carryPauseLevel[i]-1
                                        ),
                                        true));
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
    	if (squished[i])
		{
			continue;
		}
        if (oldGuyList[i].getPauseLevel() != 0)
        {
            nextGuy.push_back(
                ObjectAndTime<Guy>(
                    oldGuyList[i],
                    time->nextFrame(oldGuyList[i].getTimeDirection())));
        }
        else if (oldGuyList[i].getIndex() < playerInput.size())
        {
            std::size_t relativeIndex(oldGuyList[i].getIndex());
            const InputList& input = playerInput[relativeIndex];
            int nextCarryPauseLevel = carryPauseLevel[i];
            int relativeToPortal = -1;

            // add departure for guy at the appropriate time
            TimeDirection nextTimeDirection = oldGuyList[i].getTimeDirection();
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
                nextTime = time->entryChildFrame(pauseID, oldGuyList[i].getTimeDirection());

                pauseTimes.push_back(pauseID);
            }
            else if (input.getUse() == true)
            {
                int mx = x[i] + oldGuyList[i].getWidth() / 2;
                int my = y[i] + oldGuyList[i].getHeight() / 2;
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

            if (playerInput.size() - 1 == relativeIndex)
            {
                currentPlayerFrame = true;
                //cout << "nextPlayerFrame set to: " << nextPlayerFrame.frame() << "  " << x[i] << "\n";
            }

            nextGuy.push_back(
                ObjectAndTime<Guy>(
                    Guy(
                        x[i], y[i], xspeed[i], yspeed[i],
                        oldGuyList[i].getWidth(), oldGuyList[i].getHeight(),
                        relativeToPortal, supported[i], supportedSpeed[i], facing[i],
                        carry[i], carrySize[i], carryDirection[i], nextCarryPauseLevel,
                        nextTimeDirection, 0,
                        relativeIndex+1),
                    nextTime));
        }
        else
        {
            assert(oldGuyList[i].getIndex() == playerInput.size());
            nextPlayerFrame = true;
        }
    }
}
}//namespace
namespace {
bool explodeBoxes(
    std::vector<int>& pos,
    const std::vector<int>& size,
    const std::vector<std::vector<int> >& links,
    std::vector<char>& toBeSquished,
    const std::vector<int>& bound,
    std::size_t index,
    int boundSoFar,
    int sign)
{
	// sign = 1, small to large (eg left to right)
	// sign = -1, large to small (eg right to left)
	pos[index] = boundSoFar;

	bool subSquished = false;

	for (std::size_t i(0), isize(links[index].size()); i < isize; ++i)
	{
		subSquished = 
            explodeBoxes(
                pos,
                size,
                links,
                toBeSquished,
                bound,
                links[index][i],
                boundSoFar + size[index] * sign,
                sign) 
            || subSquished;
	}

	if (subSquished || (bound[index] != 0 && bound[index] * sign <= boundSoFar * sign))
	{
		toBeSquished[index] = true;
		return true;
    }
	return false;
}

bool explodeBoxesUpwards(
    std::vector<int>& x,
    const std::vector<int>& xTemp,
    std::vector<int>& y,
    const std::vector<int>& size,
    const std::vector<std::vector<int> >& links,
    bool firstTime,
    std::vector<char>& toBeSquished,
    const std::vector<int>& bound,
    std::size_t index,
    int boundSoFar)
{
	y[index] = boundSoFar;
	boundSoFar = boundSoFar - size[index];

	bool subSquished = false;

	for (unsigned int i = 0; i < links[index].size(); ++i)
	{
		if (firstTime)
		{
            // boxes sitting on this one
			x[links[index][i]] = xTemp[links[index][i]] + x[index] - xTemp[index];
		}
		else if (x[index] != xTemp[index])
		{
            // boxes sitting on this one
			x[links[index][i]] = xTemp[links[index][i]] + x[index] - xTemp[index];
		}
		subSquished = 
            explodeBoxesUpwards(
                x,
                xTemp,
                y,
                size,
                links,
                firstTime,
                toBeSquished,
                bound,
                links[index][i],
                boundSoFar)
            || subSquished;
	}

	if (subSquished || (bound[index] != 0 && bound[index] >= boundSoFar))
	{
		toBeSquished[index] = true;
		return true;
	}
	return false;
}

void recursiveBoxCollision(
    std::vector<int>& majorAxis,
    const std::vector<int>& minorAxis,
    const std::vector<int>& size,
    const std::vector<char>& squished,
    std::vector<int>& boxesSoFar,
    std::size_t index)
{
	boxesSoFar.push_back(index);

	for (std::size_t i(0), isize(majorAxis.size()); i < isize; ++i)
	{
		if (i != index && !squished[i] &&
			IntersectingRectanglesExclusive(
                majorAxis[index], minorAxis[index], size[index], size[index],
                majorAxis[i], minorAxis[i], size[i], size[i]) &&
			std::abs(majorAxis[index] - majorAxis[i]) > std::abs(minorAxis[index] - minorAxis[i]))
		{
			int overlap = -(majorAxis[index] + size[index] - majorAxis[i]); // index must move UP
			if (majorAxis[i] < majorAxis[index])
			{
				overlap = majorAxis[i] + size[i] - majorAxis[index];  // index must move DOWN
			}

			int indexMovement = overlap/(static_cast<int>(boxesSoFar.size()) + 1);
			int iMovement = -overlap + indexMovement;
			for (std::size_t j(0), jsize(boxesSoFar.size()); j < jsize; ++j)
			{
				majorAxis[boxesSoFar[j]] = majorAxis[boxesSoFar[j]] + indexMovement;
			}
			majorAxis[i] = majorAxis[i] + iMovement;

			recursiveBoxCollision(majorAxis, minorAxis, size, squished, boxesSoFar, i);
		}
	}
}

template <
    typename RandomAccessBoxRange,
    typename RandomAccessPlatformRange>
void boxCollisionAlogorithm(
    const Environment& env,
    const RandomAccessBoxRange& oldBoxList,
    std::vector<ObjectAndTime<Box> >& nextBox,
    RandomAccessPlatformRange& nextPlatform,
    Frame* time)
{

	std::vector<int> x(oldBoxList.size());
	std::vector<int> y(oldBoxList.size());
	std::vector<int> xTemp(oldBoxList.size());
	std::vector<int> yTemp(oldBoxList.size());
	std::vector<int> size(oldBoxList.size());
	std::vector<char> squished(oldBoxList.size(), false);

	// Destroy boxes that are overlapping, deals with chronofrag (maybe too strictly?)
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		if (!squished[i])
		{
			for (std::size_t j(0), jsize(boost::distance(oldBoxList)); j < jsize; ++j)
			{
				if (j != i && !squished[j])
				{
					int sizeI = oldBoxList[i].getSize();
					int sizeJ = oldBoxList[j].getSize();
					if (IntersectingRectanglesExclusive(oldBoxList[i].getX() + sizeI/4, oldBoxList[i].getY() + sizeI/4, sizeI/2, sizeI/2,
					oldBoxList[j].getX() + sizeJ/4, oldBoxList[j].getY() + sizeJ/4, sizeJ/2, sizeJ/2))
					{
						squished[i] = true;
						squished[j] = true;
					}
				}
			}
		}
	}

	// Make a list of pause boxes, these are collided with like platforms.
	std::vector<const Box*> pauseBoxes;
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		if (oldBoxList[i].getPauseLevel() != 0)
		{
			pauseBoxes.push_back(&oldBoxList[i]);
			squished[i] = true; // squished is equivelent to paused as they do not do things
		}
		else
		{
			xTemp[i] = oldBoxList[i].getX();
			yTemp[i] = oldBoxList[i].getY();
			x[i] = oldBoxList[i].getX() + oldBoxList[i].getXspeed();
			y[i] = oldBoxList[i].getY() + oldBoxList[i].getYspeed() + env.gravity;
			size[i] = oldBoxList[i].getSize();
		}
	}

	// do all the other things until there are no more things to do
	bool thereAreStillThingsToDo = true;
	bool firstTimeThrough = true;
	while (thereAreStillThingsToDo)
	{
		std::vector<int> top(oldBoxList.size(), 0);
		std::vector<int> bottom(oldBoxList.size(), 0);
		std::vector<int> left(oldBoxList.size(), 0);
		std::vector<int> right(oldBoxList.size(), 0);

		std::vector<std::vector<int> > topLinks(oldBoxList.size());
		std::vector<std::vector<int> > bottomLinks(oldBoxList.size());
		std::vector<std::vector<int> > rightLinks(oldBoxList.size());
		std::vector<std::vector<int> > leftLinks(oldBoxList.size());

		thereAreStillThingsToDo = false;

		// collide boxes with platforms, walls and paused boxes to discover the hard bounds on the system
		// if a box moves thereAreStillThingsToDo
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				topLinks[i] = std::vector<int>();
				bottomLinks[i] = std::vector<int>();
				rightLinks[i] = std::vector<int>();
				leftLinks[i] = std::vector<int>();

				// Check inside a wall, velocity independent which is why it is so complex
				bool topRightDiagonal = (y[i] - (y[i]/env.wall.segmentSize())*env.wall.segmentSize()) < (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize());
				bool topLeftDiagonal = (y[i] - (y[i]/env.wall.segmentSize())*env.wall.segmentSize()) + (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize()) < env.wall.segmentSize();
				if (env.wall.at(x[i], y[i])) // top left
				{
					if (env.wall.at(x[i]+size[i], y[i]) || !(env.wall.at(x[i], y[i]+size[i]) || topRightDiagonal)) // top right
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = y[i];
						if (env.wall.at(x[i], y[i]+size[i])) // bottom left
						{
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = x[i];
						}
					}
					else // bottom left
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = x[i];
						if (env.wall.at(x[i]+size[i], y[i]+size[i])) // bottom right
						{
							y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = y[i];
							x[i] = xTemp[i];
						}
					}

				}
				else if (env.wall.at(x[i], y[i]+size[i])) // bottom left and not top left
				{
					if (!(topLeftDiagonal || env.wall.at(x[i]+size[i], y[i]+size[i])))
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = x[i];
						if (env.wall.at(x[i]+size[i], y[i]+size[i])) // bottom right
						{
							y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = y[i];
							x[i] = xTemp[i];
						}
					}
					else
					{
						y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = y[i];
						x[i] = xTemp[i];
						if (env.wall.at(x[i]+size[i], y[i])) // top right
						{
							x[i] = ((x[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							right[i] = x[i];
						}
					}

				}
				else if (env.wall.at(x[i]+size[i], y[i]+size[i])) // no left and bottom right
				{
					if (!env.wall.at(x[i]+size[i], y[i]) && topRightDiagonal)
					{
						y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = y[i];
						x[i] = xTemp[i];
					}
					else
					{
						x[i] = ((x[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = x[i];
					}
				}
				else if (env.wall.at(x[i]+size[i], y[i])) // only top right
				{
					if (!topRightDiagonal)
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = y[i];
					}
					else
					{
						x[i] = ((x[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = x[i];
					}
				}

				// Inside paused box
				for (std::size_t j(0), jsize(pauseBoxes.size()); j < jsize; ++j)
				{
					int boxX = pauseBoxes[j]->getX();
					int boxY = pauseBoxes[j]->getY();
					int boxSize = pauseBoxes[j]->getSize();
					if (IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], boxX, boxY, boxSize, boxSize))
					{
						if (std::abs(x[i] - boxX) < std::abs(y[i] - boxY)) // top or bot
						{
							if (y[i] < boxY) // box above platform
							{
								y[i] = boxY - size[i];
								bottom[i] = y[i];
								x[i] = xTemp[i];
							}
							else // box below platform
							{
								y[i] = boxY + boxSize;
								top[i] = y[i];
							}
						}
						else // left or right
						{
							if (x[i] < boxX) // box left of platform
							{
								x[i] = boxX - size[i];
								right[i] = x[i];
							}
							else // box right of platform
							{
								x[i] = boxX + boxSize;
								left[i] = x[i];
							}
						}
					}
				}

				// Check inside a platform
				foreach (const Platform& platform, nextPlatform)
				{
					int pX(platform.getX());
					int pY(platform.getY());
					TimeDirection pDirection(platform.getTimeDirection());
					if (pDirection * oldBoxList[i].getTimeDirection() == hg::REVERSE && platform.getPauseLevel() == 0)
					{
						pX -= platform.getXspeed();
						pY -= platform.getYspeed();
					}
					int pWidth(platform.getWidth());
					int pHeight(platform.getHeight());

					if (IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], pX, pY, pWidth, pHeight))
					{
						if (IsPointInVerticalQuadrant(x[i] + size[i]/2, y[i] + size[i]/2, pX, pY, pWidth, pHeight))
						{
							if (y[i] + size[i]/2 < pY + pHeight/2) // box above platform
							{
								y[i] = pY - size[i];
								bottom[i] = y[i];
								if (platform.getPauseLevel() == 0 && firstTimeThrough)
								{
									x[i] = xTemp[i] + pDirection * oldBoxList[i].getTimeDirection() * platform.getXspeed();
								}
								else
								{
									x[i] = xTemp[i];
								}
							}
							else
							{
								y[i] = pY + pHeight;
								top[i] = y[i];
							}
						}
						else // left or right
						{
							if (x[i] + size[i]/2 < pX + pWidth/2) // box left of platform
							{
								x[i] = pX - size[i];
								right[i] = x[i];
							}
							else
							{
								x[i] = pX + pWidth;
								left[i] = x[i];
							}
						}
					}
				}


			}
		}

		// Now make the map of vertical collisions
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				for (std::size_t j(0); j < i; ++j)
				{
					if (j != i && !squished[j])
					{
						if (IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], x[j], y[j], size[j], size[j]))
						{
							if (std::abs(x[i] - x[j]) < std::abs(y[i] - y[j])) // top or bot
							{
								if (y[i] < y[j]) // i above j
								{
									bottomLinks[i].push_back(j);
									topLinks[j].push_back(i);
								}
								else // i below j
								{
									topLinks[i].push_back(j);
									bottomLinks[j].push_back(i);
								}
							}
							else // left or right
							{
								//if (x[i] < x[j]) // i left of j
								//{
								//	rightLinks[i].push_back(j);
								//	leftLinks[j].push_back(i);
								//}
								//else // i right of j
								//{
								//	leftLinks[i].push_back(j);
								//	rightLinks[j].push_back(i);
								//}
							}
						}
					}
				}
			}
		}

		// propagate through vertical collision links to reposition and explode
		std::vector<char> toBeSquished(oldBoxList.size(), false);

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (bottom[i] != 0)
				{
					explodeBoxesUpwards(x, xTemp, y, size, topLinks, firstTimeThrough, toBeSquished, top, i, bottom[i]);
				}
				if (top[i] != 0)
				{
					explodeBoxes(y, size, bottomLinks, toBeSquished, bottom, i, top[i], 1);
				}
			}
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (toBeSquished[i])
			{
				//cout << "vertical" << endl;
				squished[i] = true;
			}
		}

		// Now make the map of horizontal collisions
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				for (unsigned int j = 0; j < i; ++j)
				{
					if (j != i && !squished[j])
					{
						if (IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], x[j], y[j], size[j], size[j]))
						{
							if (std::abs(x[i] - x[j]) < std::abs(y[i] - y[j])) // top or bot
							{
								//if (y[i] < y[j]) // i above j
								//{
								//	bottomLinks[i].push_back(j);
								//	topLinks[j].push_back(i);
								//}
								//else // i below j
								//{
								//	topLinks[i].push_back(j);
								//	bottomLinks[j].push_back(i);
								//}
							}
							else // left or right
							{
								if (x[i] < x[j]) // i left of j
								{
									rightLinks[i].push_back(j);
									leftLinks[j].push_back(i);
								}
								else // i right of j
								{
									leftLinks[i].push_back(j);
									rightLinks[j].push_back(i);
								}
							}
						}
					}
				}
			}
		}

		// propagate through horizontal collision links to reposition and explode
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			toBeSquished[i] = false;
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (right[i] != 0)
				{
					explodeBoxes(x, size, leftLinks, toBeSquished, left, i, right[i], -1);
				}
				if (left[i] != 0)
				{
					explodeBoxes(x, size, rightLinks, toBeSquished, right, i, left[i], 1);
				}
			}
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (toBeSquished[i])
			{
				//cout << "horizontal" << endl;
				squished[i] = true;
			}
		}

		// Do something recusive!
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				std::vector<int> pass(0);
				recursiveBoxCollision(y, x, size, squished, pass, i);
			}
		}

		// And now in that other dimension!
		for (long i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				std::vector<int> pass(0);
				recursiveBoxCollision(x, y, size, squished, pass, i);
			}
		}

		// Check if I must do what Has To Be Done (again)
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (x[i] != xTemp[i] || y[i] != yTemp[i])
				{
					thereAreStillThingsToDo = true;
					xTemp[i] = x[i];
					yTemp[i] = y[i];
				}
			}
		}

		firstTimeThrough = false;
	}

	// get this junk out of here
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
        //Boxes cant be squished in pause time (right?)
        if (oldBoxList[i].getPauseLevel() != 0) {
            nextBox.push_back
			(
				ObjectAndTime<Box>(
					Box(oldBoxList[i]),
					time->nextFrame(oldBoxList[i].getTimeDirection())
				)
			);
        }
		else if (!squished[i])
		{
			nextBox.push_back
			(
				ObjectAndTime<Box>(
					Box(
						x[i],
						y[i],
						x[i] - oldBoxList[i].getX(),
						y[i] - oldBoxList[i].getY(),
						size[i],
						oldBoxList[i].getTimeDirection(),
						0),
					time->nextFrame(oldBoxList[i].getTimeDirection())
				)
			);
		}
	}

}
bool wallAtInclusive(const Environment& env, int x, int y, int w, int h)
{
    return env.wall.at(x, y) || env.wall.at(x+w, y) || env.wall.at(x, y+h) || env.wall.at(x+w, y+h);
}
bool wallAtExclusive(const Environment& env, int x, int y, int w, int h)
{
    return env.wall.at(x+1, y+1) || env.wall.at(x+w-1, y+1) || env.wall.at(x+1, y+h-1) || env.wall.at(x+w-1, y+h-1);
}
bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h)
{
    return
        (px <= x + w && px >= x)
        &&
        (py <= y + h && py >= y)
        ;
}

bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h)
{
	if (w > h)
	{
		if (x < x1 + h/2) // left
		{
			return std::abs(x - (x1 + h/2)) < std::abs(y - (y1 + h/2));
		}
		else if  (x < x1 + w - h/2) // middle
		{
			return true;
		}
		else // right
		{
			return std::abs(x - (x1 + w - h/2)) < std::abs(y - (y1 + h/2));
		}
	}
	else
	{
		if (y < y1 + w/2) // top
		{
			return std::abs(x - (x1 + w/2)) < std::abs(y - (y1 + w/2));
		}
		else if  (x < x1 + w - h/2) // middle
		{
			return false;
		}
		else // bottom
		{
			return std::abs(x - (x1 + w/2)) < std::abs(y - (y1 + h - w/2));
		}
	}
}

bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
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

bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
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
} //namespace
} //namespace hg
