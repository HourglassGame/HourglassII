#include "PhysicsEngine.h"

#include "Frame.h"
#include "Universe.h"

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/foreach.hpp>

#include "mt/std/map"
#include "mt/std/vector"

#include <cassert>
#include <cmath>

#define foreach BOOST_FOREACH

namespace hg {
enum { JUMP_SPEED 	= -550 };

PhysicsEngine::PhysicsEngine(
    Environment const& env,
    NewOldTriggerSystem const& newTriggerSystem) :
        env_(env),
        newTriggerSystem_(newTriggerSystem)
{
}
namespace {
    template<typename Object>
    struct ObjectAndTime
    {
        ObjectAndTime(
            Object const& nobject,
            Frame* nTime) :
                object(nobject),
                time(nTime) {}
        Object object;
        Frame* time;
    };
    struct SortObjectList
    {
        void operator()(std::pair<Frame* const, ObjectList<Normal> >& toSortObjectListOf) const
        {
            toSortObjectListOf.second.sort();
        }
    };
    
    template<typename RandomAccessGuyRange>
    void guyStep(
        Environment const& env,
        RandomAccessGuyRange const& guyArrivalList,
        Frame* time,
        std::vector<InputList> const& playerInput,
        mt::std::vector<ObjectAndTime<Guy> >::type& nextGuy,
        mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
        mt::std::vector<Collision>::type const& nextPlatform,
        mt::std::vector<PortalArea>::type const& nextPortal,
        bool& currentPlayerFrame,
        bool& nextPlayerFrame,
        bool& winFrame);
    
    template <
        typename RandomAccessBoxRange,
        typename RandomAccessPortalRange,
        typename RandomAccessPlatformRange>
    void boxCollisionAlogorithm(
        const Environment& env,
        const RandomAccessBoxRange& oldBoxList,
        mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
        const RandomAccessPlatformRange& nextPlatform,
        const RandomAccessPortalRange& nextPortal,
        Frame* time);
    
    template <
		typename RandomAccessPortalRange>
    void makeBoxAndTimeWithPortals(
		mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
		const RandomAccessPortalRange& nextPortal,
		int x,
		int y,
		int xspeed,
		int yspeed,
		int size,
		int oldIllegalPortal,
		TimeDirection oldTimeDirection,
		Frame* time);

    bool explodeBoxesUpwards(
        mt::std::vector<int>::type& x,
        mt::std::vector<int>::type const& xTemp,
        mt::std::vector<int>::type& y,
        mt::std::vector<int>::type const& size,
        mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
        bool firstTime,
        mt::std::vector<char>& toBeSquished,
        mt::std::vector<int> const& bound,
        std::size_t index,
        int boundSoFar);

    bool explodeBoxes(
        mt::std::vector<int>::type& pos,
        mt::std::vector<int>::type const& size,
        mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
        mt::std::vector<char>::type& toBeSquished,
        mt::std::vector<int>::type const& bound,
        std::size_t index,
        int boundSoFar,
        int sign);

    void recursiveBoxCollision(
        mt::std::vector<int>::type& majorAxis,
        mt::std::vector<int>::type const& minorAxis,
        mt::std::vector<int>::type const& size,
        mt::std::vector<char>::type const& squished,
        mt::std::vector<std::size_t>::type& boxesSoFar,
        std::size_t index);
        
    template <typename Type>
    void buildDeparturesForComplexEntities(
        typename mt::std::vector<ObjectAndTime<Type> >::type const& next,
        PhysicsEngine::FrameDepartureT& newDepartures);
        
    void buildDepartures(
        mt::std::vector<ObjectAndTime<Box> >::type const& nextBox,
        mt::std::vector<ObjectAndTime<Guy> >::type const& nextGuy,
        PhysicsEngine::FrameDepartureT& newDepartures,
        Frame* time);
        
    //bool wallAtInclusive(const Environment& env, int x, int y, int w, int h);
    bool wallAtExclusive(const Environment& env, int x, int y, int w, int h);
        
    bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h);
    bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
    bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
}


PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
    ObjectPtrList<Normal> const& arrivals,
    Frame* time,
    std::vector<InputList> const& playerInput) const
{
    NewOldTriggerFrameState triggerFrameState(newTriggerSystem_.getFrameState());
    
    //{extra boxes, collision, death, portal, pickup, arrival location}    
    PhysicsAffectingStuff const physicsTriggerStuff(
        triggerFrameState.calculatePhysicsAffectingStuff(arrivals.getList<TriggerData>()));

    mt::std::vector<ObjectAndTime<Box> >::type nextBox;

    // boxes do their crazy wizz-bang collision algorithm
    boxCollisionAlogorithm(
        env_,
        arrivals.getList<Box>(),
        nextBox,
        physicsTriggerStuff.collisions/*staticDepartures.getList<Platform>()*/,
        physicsTriggerStuff.portals/*staticDepartures.getList<Portal>()*/,
        time);

    bool currentPlayerFrame(false);
    bool nextPlayerFrame(false);
    bool winFrame(false);

    mt::std::vector<ObjectAndTime<Guy> >::type nextGuy;
    
    FrameDepartureT newDepartures;

    // guys simple collision algorithm
    guyStep(
        env_,
        arrivals.getList<Guy>(),
        time,
        playerInput,
        nextGuy,
        nextBox,
        physicsTriggerStuff.collisions,
        physicsTriggerStuff.portals,
        currentPlayerFrame,
        nextPlayerFrame,
        winFrame);

    buildDepartures(
        nextBox,
        nextGuy,
        newDepartures,
        time);

    //Sort all object lists before returning to other code. They must be sorted for comparisons to work correctly.
    boost::for_each(newDepartures, SortObjectList());
    typedef mt::std::map<
                Frame*,
                mt::std::vector<TriggerData>::type>::type triggerDepartures_t;
    std::pair<
        triggerDepartures_t,
        mt::std::vector<RectangleGlitz>::type
    > triggerDeparturesAndGlitz(
        triggerFrameState.getTriggerDeparturesAndGlitz(
            newDepartures,
            time));
    typedef triggerDepartures_t::value_type triggerDeparture_t;
    foreach (triggerDeparture_t const& triggerDeparture, triggerDeparturesAndGlitz.first) {
        //Should probably move triggerDeparture.second into newDepartures, rather than copy it.
        newDepartures[triggerDeparture.first].addRange(triggerDeparture.second);
    }

    //also sort trigger departures. TODO: do this better (ie, don't re-sort non-trigger departures).
    boost::for_each(newDepartures, SortObjectList());
    // add data to departures
    return PhysicsReturnT(newDepartures, triggerDeparturesAndGlitz.second, currentPlayerFrame, nextPlayerFrame, winFrame);
}

namespace {
template<typename Type>
void buildDeparturesForComplexEntities(
    typename mt::std::vector<ObjectAndTime<Type> >::type const& next,
    PhysicsEngine::FrameDepartureT& newDepartures)
{
    foreach (ObjectAndTime<Type> const& thingAndTime, next)
    {
        newDepartures[thingAndTime.time].add(thingAndTime.object);
    }
}

void buildDepartures(
    mt::std::vector<ObjectAndTime<Box> >::type const& nextBox,
    mt::std::vector<ObjectAndTime<Guy> >::type const& nextGuy,
    PhysicsEngine::FrameDepartureT& newDepartures,
    Frame* time)
{
    (void)time;
    buildDeparturesForComplexEntities<Box>(nextBox, newDepartures);
    buildDeparturesForComplexEntities<Guy>(nextGuy, newDepartures);
}
}//namespace

namespace {
template<typename RandomAccessGuyRange>
void guyStep(
    Environment const& env,
    RandomAccessGuyRange const& guyArrivalList,
    Frame* time,
    std::vector<InputList> const& playerInput,
    mt::std::vector<ObjectAndTime<Guy> >::type& nextGuy,
    mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
    mt::std::vector<Collision>::type const& nextPlatform,
    mt::std::vector<PortalArea>::type const& nextPortal,
    bool& currentPlayerFrame,
    bool& nextPlayerFrame,
    bool& winFrame)
{
    mt::std::vector<int>::type x;
    mt::std::vector<int>::type y;
    mt::std::vector<int>::type xspeed;
    mt::std::vector<int>::type yspeed;
    mt::std::vector<char>::type supported;
    mt::std::vector<int>::type supportedSpeed;
    mt::std::vector<char>::type squished;
    mt::std::vector<char>::type facing;

    x.reserve(boost::distance(guyArrivalList));
    y.reserve(boost::distance(guyArrivalList));
    xspeed.reserve(boost::distance(guyArrivalList));
    yspeed.reserve(boost::distance(guyArrivalList));
    supported.reserve(boost::distance(guyArrivalList));
    supportedSpeed.reserve(boost::distance(guyArrivalList));
    squished.reserve(boost::distance(guyArrivalList));
    facing.reserve(boost::distance(guyArrivalList));

    // position, velocity, collisions
    // check collisions in Y direction then do the same in X direction
    for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
    {
        if (guyArrivalList[i].getRelativeToPortal() == -1)
        {
            x.push_back(guyArrivalList[i].getX());
            y.push_back(guyArrivalList[i].getY());
        }
        else
        {
            PortalArea const& relativePortal(nextPortal[guyArrivalList[i].getRelativeToPortal()]);
            x.push_back(relativePortal.getX() + guyArrivalList[i].getX());
            y.push_back(relativePortal.getY() + guyArrivalList[i].getY());
        }
        xspeed.push_back(0);
        yspeed.push_back(guyArrivalList[i].getYspeed() + env.gravity);
        supported.push_back(false);
        supportedSpeed.push_back(0);
        squished.push_back(false);
        facing.push_back(guyArrivalList[i].getFacing());

        if (guyArrivalList[i].getIndex() < playerInput.size())
        {
            std::size_t relativeIndex(guyArrivalList[i].getIndex());
            InputList const& input(playerInput[relativeIndex]);

            int const width(guyArrivalList[i].getWidth());
            int const height(guyArrivalList[i].getHeight());

            // chonofrag with platforms
			foreach (const Collision& platform, nextPlatform)
			{
				int pX(platform.getX());
				int pY(platform.getY());
				TimeDirection pDirection(platform.getTimeDirection());
				if (pDirection * guyArrivalList[i].getTimeDirection() == hg::FORWARDS)
				{
					pX -= platform.getXspeed();
					pY -= platform.getYspeed();
				}
				int pWidth(platform.getWidth());
				int pHeight(platform.getHeight());

				if (IntersectingRectanglesExclusive(x[i], y[i], width, height, pX, pY, pWidth, pHeight))
				{
					squished[i] = true;
					continue;
				}
			}

            bool bottom = false;
            bool top = false;
            bool left = false;
            bool right = false;

            std::size_t boxThatIamStandingOn(std::numeric_limits<std::size_t>::max());

            // jump
            if (guyArrivalList[i].getSupported() && input.getUp())
            {
                yspeed[i] = guyArrivalList[i].getSupportedSpeed() + JUMP_SPEED;
            }

            // Y direction collisions
            int newY(y[i] + yspeed[i]);

            // box collision (only occurs in Y direction)
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
					if (boxDirection * guyArrivalList[i].getTimeDirection() == hg::REVERSE)
					{
						if (newY + height >= boxY-boxYspeed && newY + height-yspeed[i] <= boxY)
						{
							boxThatIamStandingOn = j;
							newY = boxY-height-boxYspeed;
							xspeed[i] = -boxXspeed;
							supported[i] = true;
							bottom = true;
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
							bottom = true;
							supportedSpeed[i] = boxYspeed;
						}
					}
				}
			}

            // check platform collision in Y direction
            foreach (const Collision& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection * guyArrivalList[i].getTimeDirection() == hg::REVERSE)
                {
                    pX -= platform.getXspeed();
                    pY -= platform.getYspeed();
                }
                int pWidth(platform.getWidth());
                int pHeight(platform.getHeight());

                if (IntersectingRectanglesExclusive(
                        x[i], newY, width, height,
                		pX - pDirection * guyArrivalList[i].getTimeDirection() * platform.getXspeed(), pY, pWidth, pHeight))
                {
                    if (newY+height/2 < pY+pHeight/2)
                    {
                        newY = pY-height;
                        xspeed[i] = pDirection * guyArrivalList[i].getTimeDirection() * platform.getXspeed();
                        supported[i] = true;
                        bottom = true;
                        supportedSpeed[i] = pDirection * guyArrivalList[i].getTimeDirection() * platform.getYspeed();
                    }
                    else
                    {
                        newY = pY + pHeight;
                        top = true;
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
					bottom = true;
					supportedSpeed[i] = 0;
				}
			}
			else if (yspeed[i] < 0) // up
			{
				if  (env.wall.at(x[i], newY) || (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-width && env.wall.at(x[i]+width, newY)))
				{
					newY = (newY/env.wall.segmentSize() + 1)*env.wall.segmentSize();
					top = true;
				}
			}

            if (bottom && top)
            {
            	squished[i] = true;
            	continue;
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

            // platform collision
            foreach (const Collision& platform, nextPlatform)
            {
                int pX(platform.getX());
                int pY(platform.getY());
                TimeDirection pDirection(platform.getTimeDirection());
                if (pDirection*guyArrivalList[i].getTimeDirection() == hg::REVERSE)
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
                        right = true;
                    }
                    else
                    {
                        newX = pX+pWidth;
                        left = true;
                    }
                }
            }

            if (newX-x[i] > 0) // right
			{
				if (env.wall.at(newX+width, newY) || (newY - (newY/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-height && env.wall.at(newX+width, newY+height)))
				{
					newX = (newX+width)/env.wall.segmentSize()*env.wall.segmentSize() - width;
					right = true;
				}
			}
			else if (newX-x[i] < 0) // left
			{
				if (env.wall.at(newX, newY) || (newY - (newY/env.wall.segmentSize())*env.wall.segmentSize() > env.wall.segmentSize()-height && env.wall.at(newX, newY+height)))
				{
					newX = (newX/env.wall.segmentSize() + 1)*env.wall.segmentSize();
					left = true;
				}
			}

            if (left && right)
			{
				squished[i] = true;
				continue;
			}

			// Apply Change
            xspeed[i] = newX-x[i];
            yspeed[i] = newY-y[i];

            x[i] = newX;
            y[i] = newY;
        }
    }
    assert(boost::distance(x) == boost::distance(guyArrivalList));
    assert(boost::distance(y) == boost::distance(guyArrivalList));
    assert(boost::distance(xspeed) == boost::distance(guyArrivalList));
    assert(boost::distance(yspeed) == boost::distance(guyArrivalList));
    assert(boost::distance(supported) == boost::distance(guyArrivalList));
    assert(boost::distance(supportedSpeed) == boost::distance(guyArrivalList));
    assert(boost::distance(squished) == boost::distance(guyArrivalList));
    assert(boost::distance(facing) == boost::distance(guyArrivalList));


    mt::std::vector<char>::type carry;
    mt::std::vector<int>::type carrySize;
    mt::std::vector<TimeDirection>::type carryDirection;
    carry.reserve(guyArrivalList.size());
    carrySize.reserve(guyArrivalList.size());
    carryDirection.reserve(guyArrivalList.size());
    // box carrying
    for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
    {
        carry.push_back(guyArrivalList[i].getBoxCarrying());
        carrySize.push_back(0);
        carryDirection.push_back(hg::INVALID);

        if (guyArrivalList[i].getIndex() < playerInput.size() && !squished[i])
        {
            std::size_t const relativeIndex(guyArrivalList[i].getIndex());
            InputList const& input(playerInput[relativeIndex]);

            if (carry[i])
            {
                bool droppable(false);
                if (input.getDown() && supported[i])
                {
                    int width(guyArrivalList[i].getWidth());
                    int dropX(facing[i] ? x[i] + width : x[i] - guyArrivalList[i].getBoxCarrySize());
                    int dropY(y[i]);
                    int dropSize(guyArrivalList[i].getBoxCarrySize());

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
                        	makeBoxAndTimeWithPortals(
                                nextBox,
                                nextPortal,
                                dropX,
                                dropY,
                                0,
                                yspeed[i],
                                dropSize,
                                -1,
                                guyArrivalList[i].getBoxCarryDirection(),
                                time);

                            carry[i] = false;
                            carrySize[i] = 0;
                            carryDirection[i] = hg::INVALID;
                        }
                    }
                }

                if (!droppable)
                {
                    carrySize[i] = guyArrivalList[i].getBoxCarrySize();
                    carryDirection[i] = guyArrivalList[i].getBoxCarryDirection();
                }
            }
            else
            {
                if (input.getDown())
                {
                    int width = guyArrivalList[i].getWidth();
                    int height = guyArrivalList[i].getHeight();
                    //CAREFUL - loop modifies nextBox
                    for (mt::std::vector<ObjectAndTime<Box> >::type::iterator nextBoxIt(nextBox.begin()), nextBoxEnd(nextBox.end()); nextBoxIt != nextBoxEnd; ++nextBoxIt)
                    {
                        int boxX = nextBoxIt->object.getX();
                        int boxY = nextBoxIt->object.getY();
                        int boxSize = nextBoxIt->object.getSize();
                        if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY))
                        {
                            carry[i] = true;
                            carrySize[i] = boxSize;
                            carryDirection[i] = nextBoxIt->object.getTimeDirection();

                            nextBoxIt = nextBox.erase(nextBoxIt);
                            nextBoxEnd = nextBox.end();
                            break;
                        }
                    }
                }
                else
                {
                    carrySize[i] = 0;
                    carryDirection[i] = hg::INVALID;
                }
            }
        }
    }
    assert(boost::distance(carry) == boost::distance(guyArrivalList));
    assert(boost::distance(carrySize) == boost::distance(guyArrivalList));
    assert(boost::distance(carryDirection) == boost::distance(guyArrivalList));

    // colliding with pickups?
    mt::std::map<int, mt::std::map<int,int>::type >::type pickups;
    // map only holds values for guys which have changed pickup count as that shouldn't happen too often
	/*for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
	{
		if (pickup not taken && colliding with pickup && lua callin returns true)
		{
			mark this pickup as taken
			pickups[i] = mt::std::map<int,int>::type(guyArrivalList[i].getPickups());
			pickups[i][pickupAreaThatICollidedWith.getType()] += 1; // because the default value of an int is 0
		}
	}*/

    // time travel
    for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
    {
    	if (squished[i])
		{
            assert(guyArrivalList[i].getIndex() != playerInput.size()
                    && "nextPlayerFrame must always be set for guys with"
                       " index == playerInput.size()");
			continue;
		}
        if (guyArrivalList[i].getIndex() < playerInput.size())
        {
            const std::size_t relativeIndex(guyArrivalList[i].getIndex());
            const InputList& input = playerInput[relativeIndex];
            int relativeToPortal = -1;
            int illegalPortal = -1;

            // add departure for guy at the appropriate time
            TimeDirection nextTimeDirection = guyArrivalList[i].getTimeDirection();
            Frame* nextTime(nextFrame(time, nextTimeDirection));
            assert(time);

            bool normalDeparture = true;

            if (input.getAbility() == hg::TIME_JUMP /* && ((!pickups[i].empty() && pickups[i][jumpNumber]) || guyArrivalList[i].getPickups()[jumpNumber]) */ )
            {
            	// decrement pickups
                nextTime = getArbitraryFrame(getUniverse(time), input.getFrameIdParam(0).getFrameNumber());
                normalDeparture = false;
            }
            else if (input.getAbility() == hg::TIME_REVERSE) // etc... from above
            {
                normalDeparture = false;
                nextTimeDirection *= -1;
                nextTime = nextFrame(time, nextTimeDirection);
                carryDirection[i] *= -1;
            }
            else if (input.getUse() == true)
            {
                const int mx = x[i] + guyArrivalList[i].getWidth() / 2;
                const int my = y[i] + guyArrivalList[i].getHeight() / 2;
                for (unsigned int j = 0; j < nextPortal.size(); ++j)
                {
                    int px = nextPortal[j].getX();
                    int py = nextPortal[j].getY();
                    int pw = nextPortal[j].getWidth();
                    int ph = nextPortal[j].getHeight();
                    if (PointInRectangleInclusive(mx, my, px, py, pw, ph) /*&&
                            nextPortal[j].getActive() && nextPortal[j].getCharges() != 0*/
                            /*&&shouldPort() TODO*/) // charges not fully implemented
                    {
                        if (nextPortal[j].getWinner())
                        {
                        	winFrame = true;
                        	nextTime = 0;
                        	break;
                        }
                        Frame* portalTime(
                            nextPortal[j].getRelativeTime() ?
                                getArbitraryFrame(
                                    getUniverse(time),
                                    getFrameNumber(time) + nextPortal[j].getTimeDestination()) :
                                getArbitraryFrame(
                                    getUniverse(time),
                                    nextPortal[j].getTimeDestination()));
						nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : 0;
						normalDeparture = false;
						illegalPortal = nextPortal[j].getIllegalDestination();
						relativeToPortal = nextPortal[j].getDestinationIndex();
						x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination();
						y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination();
						break;
                    }
                }
            }

            // falling through portals
            for (unsigned int j = 0; j < nextPortal.size(); ++j)
			{
            	int px = nextPortal[j].getX();
				int py = nextPortal[j].getY();
				int pw = nextPortal[j].getWidth();
				int ph = nextPortal[j].getHeight();
                
				if (PointInRectangleInclusive(
                        x[i] + guyArrivalList[i].getWidth()/2, y[i] + guyArrivalList[i].getHeight()/2,
                        px, py, pw, ph)
					/*&& nextPortal[j].getActive() && nextPortal[j].getCharges() != 0 */
                    /*&& shouldPort() TODO */
                    && nextPortal[j].getFallable())
				{
					if (guyArrivalList[i].getIllegalPortal() != -1 && j == static_cast<unsigned int>(guyArrivalList[i].getIllegalPortal()))
					{
						illegalPortal = j;
					}
					else
					{
						Frame* portalTime(
                            nextPortal[j].getRelativeTime() ?
                            getArbitraryFrame(
                                getUniverse(time),
                                getFrameNumber(time) + nextPortal[j].getTimeDestination()):
                            getArbitraryFrame(
                                getUniverse(time),
                                nextPortal[j].getTimeDestination()));
						nextTime = portalTime ? nextFrame(portalTime, nextTimeDirection) : 0;
						normalDeparture = false;
						illegalPortal = nextPortal[j].getIllegalDestination();
						relativeToPortal = nextPortal[j].getDestinationIndex();
						x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination();
						y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination();
						break;
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
                        x[i], y[i],
                        xspeed[i], yspeed[i],
                        guyArrivalList[i].getWidth(), guyArrivalList[i].getHeight(),
                        
                        illegalPortal,
                        relativeToPortal,
                        supported[i],
                        supportedSpeed[i],

                        pickups[i].empty() ? guyArrivalList[i].getPickups() : pickups[i],
                        facing[i],
                        
                        carry[i],
                        carrySize[i],
                        carryDirection[i],
                        
                        nextTimeDirection,
                        relativeIndex + 1
                    ),
                    nextTime
                )
            );
        }
        else
        {
            assert(guyArrivalList[i].getIndex() == playerInput.size());
            nextPlayerFrame = true;
        }
    }
}

template <
    typename RandomAccessPortalRange>
void makeBoxAndTimeWithPortals(
		mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
		const RandomAccessPortalRange& nextPortal,
		int x,
		int y,
		int xspeed,
		int yspeed,
		int size,
		int oldIllegalPortal,
		TimeDirection oldTimeDirection,
		Frame* time)
{
	int relativeToPortal = -1;
	int illegalPortal = -1;
	Frame* nextTime(nextFrame(time, oldTimeDirection));

    for (unsigned i = 0; i < nextPortal.size(); ++i)
    {
        int px = nextPortal[i].getX();
        int py = nextPortal[i].getY();
        int pw = nextPortal[i].getWidth();
        int ph = nextPortal[i].getHeight();
        if (PointInRectangleInclusive(x + size/2, y + size/2, px, py, pw, ph)
            /*&& nextPortal[i].getActive() && nextPortal[i].getCharges() != 0*/
            /*&& shouldPort()  TODO*/
            && nextPortal[i].getFallable())
        {
            if (oldIllegalPortal != -1 && i == static_cast<unsigned>(oldIllegalPortal))
            {
                illegalPortal = i;
            }
            else
            {
                Frame* portalTime(
                    nextPortal[i].getRelativeTime() ?
                    getArbitraryFrame(getUniverse(time), getFrameNumber(time) + nextPortal[i].getTimeDestination()):
                    getArbitraryFrame(getUniverse(time), nextPortal[i].getTimeDestination()));
                nextTime = portalTime ? nextFrame(portalTime, oldTimeDirection) : 0;
                illegalPortal = nextPortal[i].getIllegalDestination();
                relativeToPortal = nextPortal[i].getDestinationIndex();
                x = x - nextPortal[i].getX() + nextPortal[i].getXdestination();
                y = y - nextPortal[i].getY() + nextPortal[i].getYdestination();
                break;
            }
        }
    }

	nextBox.push_back
	(
		ObjectAndTime<Box>(
			Box(
				x,
				y,
				xspeed,
				yspeed,
				size,
				illegalPortal,
				relativeToPortal,
				oldTimeDirection),
			nextTime
		)
	);
}

bool explodeBoxes(
    mt::std::vector<int>::type& pos,
    mt::std::vector<int>::type const& size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
    mt::std::vector<char>::type& toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const& bound,
    std::size_t index,
    int boundSoFar,
    int sign)
{
	// sign = 1, small to large (left to right / top to bottom)
	// sign = -1, large to small (right to left / bottom to top)
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

	if (subSquished || (bound[index].first && bound[index].second * sign <= boundSoFar * sign))
	{
		toBeSquished[index] = true;
		return true;
    }
	return false;
}

bool explodeBoxesUpwards(
    mt::std::vector<int>::type& x,
    mt::std::vector<int>::type const& xTemp,
    mt::std::vector<int>::type& y,
    mt::std::vector<int>::type const& size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
    bool firstTime,
    mt::std::vector<char>::type& toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const& bound,
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

	if (subSquished || (bound[index].first && bound[index].second >= boundSoFar))
	{
		toBeSquished[index] = true;
		return true;
	}
	return false;
}
void recursiveBoxCollision(
    mt::std::vector<int>::type& majorAxis,
    mt::std::vector<int>::type const& minorAxis,
    mt::std::vector<int>::type const& size,
    mt::std::vector<char>::type const& squished,
    mt::std::vector<std::size_t>::type& boxesSoFar,
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
    typename RandomAccessPortalRange,
    typename RandomAccessPlatformRange>
void boxCollisionAlogorithm(
    Environment const& env,
    RandomAccessBoxRange const& oldBoxList,
    mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
    RandomAccessPlatformRange const& nextPlatform,
    RandomAccessPortalRange const& nextPortal,
    Frame* time)
{
	mt::std::vector<int>::type x(oldBoxList.size());
	mt::std::vector<int>::type y(oldBoxList.size());
	mt::std::vector<int>::type xTemp(oldBoxList.size());
	mt::std::vector<int>::type yTemp(oldBoxList.size());
	mt::std::vector<int>::type size(oldBoxList.size());
	mt::std::vector<char>::type squished(oldBoxList.size());

	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
        if (oldBoxList[i].getRelativeToPortal() == -1)
        {
            xTemp[i] = oldBoxList[i].getX();
            yTemp[i] = oldBoxList[i].getY();
        }
        else
        {
            PortalArea const& relativePortal(nextPortal[oldBoxList[i].getRelativeToPortal()]);
            xTemp[i] = relativePortal.getX() + oldBoxList[i].getX();
            yTemp[i] = relativePortal.getY() + oldBoxList[i].getY();
        }
        x[i] = xTemp[i] + oldBoxList[i].getXspeed();
        y[i] = yTemp[i] + oldBoxList[i].getYspeed() + env.gravity;
        size[i] = oldBoxList[i].getSize();
	}

	// Destroy boxes that are overlapping with platforms
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		foreach (Collision const& platform, nextPlatform)
		{
			int pX(platform.getX());
			int pY(platform.getY());
			TimeDirection pDirection(platform.getTimeDirection());
			if (pDirection * oldBoxList[i].getTimeDirection() == hg::FORWARDS)
			{
				pX -= platform.getXspeed();
				pY -= platform.getYspeed();
			}
			int pWidth(platform.getWidth());
			int pHeight(platform.getHeight());

			if (IntersectingRectanglesExclusive(xTemp[i], yTemp[i], size[i], size[i], pX, pY, pWidth, pHeight))
			{
				squished[i] = true;
			}
		}
	}

	// Destroy boxes that are overlapping, deals with chronofrag (maybe too strictly?)
	mt::std::vector<char>::type toBeSquished(oldBoxList.size());

	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		if (!squished[i])
		{
			for (std::size_t j(0); j < i; ++j)
			{
				if (!squished[j])
				{
					if (IntersectingRectanglesExclusive(xTemp[i], yTemp[i], size[i], size[i],
						xTemp[j], yTemp[j], size[j], size[j]))
					{
						toBeSquished[i] = true;
						toBeSquished[j] = true;
					}
				}
			}
		}
	}

	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		if (toBeSquished[i])
		{
			squished[i] = true;
		}
	}

	// do all the other things until there are no more things to do
	bool thereAreStillThingsToDo(true);
	bool firstTimeThrough(true);
	while (thereAreStillThingsToDo)
	{
		mt::std::vector<std::pair<bool,int> >::type top(oldBoxList.size());
		mt::std::vector<std::pair<bool,int> >::type bottom(oldBoxList.size());
		mt::std::vector<std::pair<bool,int> >::type left(oldBoxList.size());
		mt::std::vector<std::pair<bool,int> >::type right(oldBoxList.size());

		mt::std::vector<mt::std::vector<std::size_t>::type >::type topLinks(oldBoxList.size());
		mt::std::vector<mt::std::vector<std::size_t>::type >::type bottomLinks(oldBoxList.size());
		mt::std::vector<mt::std::vector<std::size_t>::type >::type rightLinks(oldBoxList.size());
		mt::std::vector<mt::std::vector<std::size_t>::type >::type leftLinks(oldBoxList.size());

		thereAreStillThingsToDo = false;

		// collide boxes with platforms and walls to discover the hard bounds on the system
		// if a box moves thereAreStillThingsToDo
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				// Check inside a wall, velocity independent which is why it is so complex
				//true iff the top-left corner of the box is in the top-right half of a wall segment
                bool topRightDiagonal(
                    (y[i] - (y[i]/env.wall.segmentSize())*env.wall.segmentSize())
                     <
                    (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize()));
                //true iff the top-left corner of the box is in the top-left half of a wall segment
				bool topLeftDiagonal(
                    (y[i] - (y[i]/env.wall.segmentSize())*env.wall.segmentSize())
                     + (x[i] - (x[i]/env.wall.segmentSize())*env.wall.segmentSize())
                     <
                    env.wall.segmentSize());
				if (env.wall.at(x[i], y[i])) // top left
				{
					if (env.wall.at(x[i]+size[i], y[i]) || !(env.wall.at(x[i], y[i]+size[i]) || topRightDiagonal)) // top right
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
						if (env.wall.at(x[i], y[i]+size[i])) // bottom left
						{
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
						}
					}
					else // bottom left
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
						if (env.wall.at(x[i]+size[i], y[i]+size[i])) // bottom right
						{
							y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = std::make_pair(true, y[i]);
							x[i] = xTemp[i];
						}
					}

				}
				else if (env.wall.at(x[i], y[i]+size[i])) // bottom left and not top left
				{
					if (!(topLeftDiagonal || env.wall.at(x[i]+size[i], y[i]+size[i])))
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
						if (env.wall.at(x[i]+size[i], y[i]+size[i])) // bottom right
						{
							y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = std::make_pair(true, y[i]);
							x[i] = xTemp[i];
						}
					}
					else
					{
						y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = std::make_pair(true, y[i]);
						x[i] = xTemp[i];
						if (env.wall.at(x[i]+size[i], y[i])) // top right
						{
							x[i] = ((x[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							right[i] = std::make_pair(true, x[i]);
						}
					}

				}
				else if (env.wall.at(x[i]+size[i], y[i]+size[i])) // no left and bottom right
				{
					if (!env.wall.at(x[i]+size[i], y[i]) && topRightDiagonal)
					{
						y[i] = ((y[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = std::make_pair(true, y[i]);
						x[i] = xTemp[i];
					}
					else
					{
						x[i] = ((x[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = std::make_pair(true, x[i]);
					}
				}
				else if (env.wall.at(x[i]+size[i], y[i])) // only top right
				{
					if (!topRightDiagonal)
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
					else
					{
						x[i] = ((x[i]+size[i])/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = std::make_pair(true, x[i]);
					}
				}

				// Check inside a platform
				foreach (Collision const& platform, nextPlatform)
				{
					int pX(platform.getX());
					int pY(platform.getY());
					TimeDirection pDirection(platform.getTimeDirection());
					if (pDirection * oldBoxList[i].getTimeDirection() == hg::REVERSE)
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
								bottom[i] = std::make_pair(true, y[i]);
								if (firstTimeThrough)
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
								top[i] = std::make_pair(true, y[i]);
							}
						}
						else // left or right
						{
							if (x[i] + size[i]/2 < pX + pWidth/2) // box left of platform
							{
								x[i] = pX - size[i];
								right[i] = std::make_pair(true, x[i]);
							}
							else
							{
								x[i] = pX + pWidth;
								left[i] = std::make_pair(true, x[i]);
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
		mt::std::vector<char>::type toBeSquished(oldBoxList.size());

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (bottom[i].first)
				{
					explodeBoxesUpwards(x, xTemp, y, size, topLinks, firstTimeThrough, toBeSquished, top, i, bottom[i].second);
				}
				if (top[i].first)
				{
					explodeBoxes(y, size, bottomLinks, toBeSquished, bottom, i, top[i].second, 1);
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
				for (unsigned j = 0; j < i; ++j)
				{
					if (j != i 
                        && !squished[j] 
                        && IntersectingRectanglesInclusive(x[i], y[i], size[i], size[i], x[j], y[j], size[j], size[j]))
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

		// propagate through horizontal collision links to reposition and explode
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			toBeSquished[i] = false;
		}

		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				if (right[i].first)
				{
					explodeBoxes(x, size, leftLinks, toBeSquished, left, i, right[i].second, -1);
				}
				if (left[i].first)
				{
					explodeBoxes(x, size, rightLinks, toBeSquished, right, i, left[i].second, 1);
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
			    mt::std::vector<std::size_t>::type pass;
				recursiveBoxCollision(y, x, size, squished, pass, i);
			}
		}

		// And now in that other dimension!
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
			    mt::std::vector<std::size_t>::type pass;
				recursiveBoxCollision(x, y, size, squished, pass, i);
			}
		}
        
		// Check if I Must do What has Tobe Done (again)
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
		if (!squished[i])
		{
			if (oldBoxList[i].getRelativeToPortal() == -1)
			{
				makeBoxAndTimeWithPortals(
                    nextBox,
                    nextPortal,
                    x[i],
                    y[i],
                    x[i] - oldBoxList[i].getX(),
                    y[i] - oldBoxList[i].getY(),
                    size[i],
                    oldBoxList[i].getIllegalPortal(),
                    oldBoxList[i].getTimeDirection(),
                    time);
			}
			else
			{
				PortalArea const& relativePortal(nextPortal[oldBoxList[i].getRelativeToPortal()]);
				makeBoxAndTimeWithPortals(
                    nextBox,
                    nextPortal,
                    x[i],
                    y[i],
                    x[i] - relativePortal.getX() - oldBoxList[i].getX(),
                    y[i] - relativePortal.getY() - oldBoxList[i].getY(),
                    size[i],
                    oldBoxList[i].getIllegalPortal(),
                    oldBoxList[i].getTimeDirection(),
                    time);
			}

		}
	}
}
#if 0
bool wallAtInclusive(Environment const& env, int x, int y, int w, int h)
{
    return env.wall.at(x, y) || env.wall.at(x+w, y) || env.wall.at(x, y+h) || env.wall.at(x+w, y+h);
}
#endif
bool wallAtExclusive(Environment const& env, int x, int y, int w, int h)
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

bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
		(x1 >= x2 && x1 + w1 <= x2 + w2)
		&&
		(y1 >= y2 && y1 + h1 <= y2 + h2)
		;
}
} //namespace
} //namespace hg
