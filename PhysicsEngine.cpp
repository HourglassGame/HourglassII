#include "PhysicsEngine.h"

#include "Frame.h"
#include "Universe.h"

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>

#include "mt/std/map"
#include "mt/std/vector"

#include "Foreach.h"

#include <cassert>



namespace hg {
enum {
	JUMP_SPEED 	= -550,
	COLLISION_BUFFER_RANGE = 100,
	HALF_COLLISION_BUFFER_RANGE = 50
};

PhysicsEngine::PhysicsEngine(
    Environment const& env,
    TriggerSystem const& triggerSystem) :
        env_(env),
        triggerSystem_(triggerSystem)
{
}
namespace {
    struct SortObjectList
    {
        void operator()(ObjectList<Normal>& toSort) const
        {
            toSort.sort();
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
        mt::std::vector<char>::type& nextBoxNormalDeparture,
        mt::std::vector<Collision>::type const& nextPlatform,
        mt::std::vector<PortalArea>::type const& nextPortal,
        mt::std::vector<ArrivalLocation>::type const& arrivalLocations,
        mt::std::vector<MutatorArea>::type const& mutators,
        TriggerFrameState& triggerFrameState,
        bool& currentPlayerFrame,
        bool& nextPlayerFrame,
        bool& winFrame);
    
    template <
        typename RandomAccessBoxRange,
        typename RandomAccessPortalRange,
        typename RandomAccessPlatformRange,
        typename RandomAccessArrivalLocationRange,
        typename RandomAccessMutatorRange>
    void boxCollisionAlogorithm(
        Environment const& env,
        RandomAccessBoxRange const& oldBoxList,
        mt::std::vector<Box>::type const& additionalBox,
        mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
        mt::std::vector<char>::type& nextBoxNormalDeparture,
        RandomAccessPlatformRange const& nextPlatform,
        RandomAccessPortalRange const& nextPortal,
        RandomAccessArrivalLocationRange const& arrivalLocations,
        RandomAccessMutatorRange const& mutators,
        TriggerFrameState& triggerFrameState,
        Frame* time);
    
    template <
		typename RandomAccessPortalRange,
		typename RandomAccessMutatorRange>
    void makeBoxAndTimeWithPortalsAndMutators(
		mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
		mt::std::vector<char>::type& nextBoxNormalDeparture,
		const RandomAccessPortalRange& nextPortal,
		const RandomAccessMutatorRange& mutators,
		int x,
		int y,
		int xspeed,
		int yspeed,
		int size,
		int oldIllegalPortal,
		TimeDirection oldTimeDirection,
		TriggerFrameState& triggerFrameState,
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
        std::size_t index,
        int subtractionNumber);
        
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
        
    unsigned int Raytrace(
    		Environment const& env,
    		int sx, int sy, int px, int py,
    		mt::std::vector<ObjectAndTime<Box> >::type box,
    		mt::std::vector<char>::type& nextBoxNormalDeparture,
    		mt::std::vector<int>::type gx, // other guy things
    		mt::std::vector<int>::type gy,
    		mt::std::vector<int>::type gw,
    		mt::std::vector<int>::type gh,
    		mt::std::vector<char>::type shootable,
    		int myIndex);

    bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h);
    bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
    bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    bool IntersectingRectanglesInclusiveCollisionOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int buffer);
    bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    FrameView makeFrameView(PhysicsEngine::FrameDepartureT const&, mt::std::vector<RectangleGlitz>::type const&);
}


PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
    ObjectPtrList<Normal> const& arrivals,
    Frame* time,
    std::vector<InputList> const& playerInput) const
{
    TriggerFrameState triggerFrameState(triggerSystem_.getFrameState());
    
    //{extra boxes, collision, death, portal, pickup, arrival location}    
    PhysicsAffectingStuff const physicsTriggerStuff(
        triggerFrameState.calculatePhysicsAffectingStuff(time, arrivals.getList<TriggerData>()));

    mt::std::vector<ObjectAndTime<Box> >::type nextBox;
    mt::std::vector<char>::type nextBoxNormalDeparture;

    // boxes do their crazy wizz-bang collision algorithm
    boxCollisionAlogorithm(
        env_,
        arrivals.getList<Box>(),
        physicsTriggerStuff.additionalBoxes,
        nextBox,
        nextBoxNormalDeparture,
        physicsTriggerStuff.collisions,
        physicsTriggerStuff.portals,
        physicsTriggerStuff.arrivalLocations,
        physicsTriggerStuff.mutators,
        triggerFrameState,
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
        nextBoxNormalDeparture,
        physicsTriggerStuff.collisions,
        physicsTriggerStuff.portals,
        physicsTriggerStuff.arrivalLocations,
        physicsTriggerStuff.mutators,
        triggerFrameState,
        currentPlayerFrame,
        nextPlayerFrame,
        winFrame);

    buildDepartures(
        nextBox,
        nextGuy,
        newDepartures,
        time);

    //Sort all object lists before returning to other code. They must be sorted for comparisons to work correctly.
    boost::for_each(newDepartures | boost::adaptors::map_values, SortObjectList());
    typedef mt::std::map<
                Frame*,
                mt::std::vector<TriggerData>::type>::type triggerDepartures_t;

    boost::tuple<
        triggerDepartures_t,
        mt::std::vector<RectangleGlitz>::type,
        mt::std::vector<ObjectAndTime<Box> >::type
    > triggerSystemDepartureInformation(
        triggerFrameState.getDepartureInformation(
            newDepartures,
            time));
    typedef triggerDepartures_t::value_type triggerDeparture_t;
    foreach (triggerDeparture_t const& triggerDeparture, triggerSystemDepartureInformation.get<0>()) {
        //Should probably move triggerDeparture.second into newDepartures, rather than copy it.
        newDepartures[triggerDeparture.first].addRange(triggerDeparture.second);
    }

    // add extra boxes to newDepartures
    buildDeparturesForComplexEntities<Box>(triggerSystemDepartureInformation.get<2>(), newDepartures);

    //also sort trigger departures. TODO: do this better (ie, don't re-sort non-trigger departures).
    boost::for_each(newDepartures | boost::adaptors::map_values, SortObjectList());
    // add data to departures
    return PhysicsReturnT(
        newDepartures,
        makeFrameView(newDepartures, triggerSystemDepartureInformation.get<1>()),
        currentPlayerFrame,
        nextPlayerFrame,
        winFrame);
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
template<typename T>
    void pushBidirectional(
        T const& obj,
        unsigned forwardsColour, unsigned reverseColour,
        mt::std::vector<Glitz>::type& forwardsGlitz,
        mt::std::vector<Glitz>::type& reverseGlitz)
{
    Glitz sameDirectionGlitz(
        obj.getX(), obj.getY(),
        obj.getWidth(), obj.getHeight(),
        forwardsColour);
    
    Glitz oppositeDirectionGlitz(
        obj.getX() - obj.getXspeed(), obj.getY() - obj.getYspeed(),
        obj.getWidth(), obj.getHeight(),
        reverseColour);
    
    forwardsGlitz.push_back(
        obj.getTimeDirection() == FORWARDS ?
        sameDirectionGlitz:oppositeDirectionGlitz);
    
    reverseGlitz.push_back(
        obj.getTimeDirection() == REVERSE ?
        sameDirectionGlitz:oppositeDirectionGlitz);
}

void pushGuyGlitz(
    Guy const& guy,
    mt::std::vector<Glitz>::type& forwardsGlitz,
    mt::std::vector<Glitz>::type& reverseGlitz)
{
    //Guys with arrivalBasis == -1 have positions that are relative
    //to an arrival basis. This means that interpreting their positions
    //as departure positions is flat out wrong.
    //Not drawing them at all is also wrong (because it causes guys to disappear).
    //The correct thing to do is to draw where they departed from.
    //This is now possible, but requires a little bit of restructuring of physics.
    //Doing this drawing right here in this way is quite silly, and is only temporary.
    if (guy.getArrivalBasis() == -1)
    {
        //Forwards View
        {
            struct PNC {
                PNC(int nx, int ny, unsigned ncolour):
                    x(nx), y(ny), colour(ncolour) {}
                int x; int y; unsigned colour;
            } const pnc = 
                guy.getTimeDirection() == FORWARDS ?
                    PNC(guy.getX(), guy.getY(), 0x96960000u) :
                    PNC(guy.getX() - guy.getXspeed(), guy.getY() - guy.getYspeed(), 0x00009600u);
            
            int const left(pnc.x);
            int const top(pnc.y);
            int const halfwidth(guy.getWidth()/2);
            int const halfheight(guy.getHeight()/2);
            int const hmid(pnc.x+halfwidth);
            
            forwardsGlitz.push_back(Glitz(left, top, guy.getWidth(), guy.getHeight(), pnc.colour));
            forwardsGlitz.push_back(
                guy.getFacing() ?
                    Glitz(hmid, top, halfwidth, halfheight, 0x32323200u) :
                    Glitz(left, top, halfwidth, halfheight, 0x32323200u));
            
            if (guy.getBoxCarrying())
            {
                forwardsGlitz.push_back(
                    Glitz(
                        hmid - guy.getBoxCarrySize()/2,
                        top - guy.getBoxCarrySize(),
                        guy.getBoxCarrySize(),
                        guy.getBoxCarrySize(),
                        guy.getBoxCarryDirection() == FORWARDS ? 
                          0x96009600u : 0x00960000u));
            }
        }
        //Reverse View
        {
            struct PNC {
                PNC(int nx, int ny, unsigned ncolour):
                    x(nx), y(ny), colour(ncolour) {}
                int x; int y; unsigned colour;
            } const pnc = 
                guy.getTimeDirection() == REVERSE ?
                    PNC(guy.getX(), guy.getY(), 0x96960000u) :
                    PNC(guy.getX() - guy.getXspeed(), guy.getY() - guy.getYspeed(), 0x00009600u);

            int const left(pnc.x);
            int const top(pnc.y);
            int const halfwidth(guy.getWidth()/2);
            int const halfheight(guy.getHeight()/2);
            int const hmid(pnc.x+halfwidth);

            reverseGlitz.push_back(Glitz(left, top, guy.getWidth(), guy.getHeight(), pnc.colour));
            reverseGlitz.push_back(
                guy.getFacing() ?
                    Glitz(hmid, top, halfwidth, halfheight, 0x32323200u) :
                    Glitz(left, top, halfwidth, halfheight, 0x32323200u));

            if (guy.getBoxCarrying())
            {
                reverseGlitz.push_back(
                    Glitz(
                        hmid - guy.getBoxCarrySize()/2,
                        top - guy.getBoxCarrySize(),
                        guy.getBoxCarrySize(),
                        guy.getBoxCarrySize(),
                        guy.getBoxCarryDirection() == REVERSE ? 
                          0x96009600u : 0x00960000u));
            }
        }
    }
}

//This is temporary, it is an adaptor to allow the old values to be used by he new interface.
//The new version should be properly integrated into guy/box step and so on.
FrameView makeFrameView(
    PhysicsEngine::FrameDepartureT const& departures,
    mt::std::vector<RectangleGlitz>::type const& glitzlist)
{
    mt::std::vector<Glitz>::type forwardsGlitz;
    mt::std::vector<Glitz>::type reverseGlitz;
    foreach (RectangleGlitz const& glitz, glitzlist) {
        pushBidirectional(glitz, glitz.getForwardsColour(), glitz.getReverseColour(), forwardsGlitz, reverseGlitz);
    }
    ObjectPtrList<Normal>  flattenedDepartures;
    foreach (ObjectList<Normal> const& objectList, departures | boost::adaptors::map_values)
    {
        flattenedDepartures.add(objectList);
    }
    flattenedDepartures.sort();
    foreach (Box const& box, flattenedDepartures.getList<Box>()) {
        if (box.getArrivalBasis() == -1) {
            pushBidirectional(box, 0xFF00FF00u, 0x00FF00FFu, forwardsGlitz, reverseGlitz);
        }
    }
    mt::std::vector<GuyOutputInfo>::type guyInfo;
    foreach (Guy const& guy, flattenedDepartures.getList<Guy>()) {
        pushGuyGlitz(guy, forwardsGlitz, reverseGlitz);
        guyInfo.push_back(GuyOutputInfo(guy.getTimeDirection()));
    }
    return FrameView(forwardsGlitz, reverseGlitz, guyInfo);
}

template<typename RandomAccessGuyRange>
void guyStep(
    Environment const& env,
    RandomAccessGuyRange const& guyArrivalList,
    Frame* time,
    std::vector<InputList> const& playerInput,
    mt::std::vector<ObjectAndTime<Guy> >::type& nextGuy,
    mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    mt::std::vector<Collision>::type const& nextPlatform,
    mt::std::vector<PortalArea>::type const& nextPortal,
    mt::std::vector<ArrivalLocation>::type const& arrivalLocations,
    mt::std::vector<MutatorArea>::type const& mutators,
    TriggerFrameState& triggerFrameState,
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
    mt::std::vector<char>::type finishedWith;
    mt::std::vector<char>::type facing;

    x.reserve(boost::distance(guyArrivalList));
    y.reserve(boost::distance(guyArrivalList));
    xspeed.reserve(boost::distance(guyArrivalList));
    yspeed.reserve(boost::distance(guyArrivalList));
    supported.reserve(boost::distance(guyArrivalList));
    supportedSpeed.reserve(boost::distance(guyArrivalList));
    finishedWith.reserve(boost::distance(guyArrivalList));
    facing.reserve(boost::distance(guyArrivalList));

    // position, velocity, collisions
    // check collisions in Y direction then do the same in X direction
    for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
    {
    	// initialise positions with arrivalBasis
        if (guyArrivalList[i].getArrivalBasis() == -1)
        {
            x.push_back(guyArrivalList[i].getX());
            y.push_back(guyArrivalList[i].getY());
            xspeed.push_back(0);
			yspeed.push_back(guyArrivalList[i].getYspeed() + env.gravity);
        }
        else
        {
            ArrivalLocation const& relativePortal(arrivalLocations[guyArrivalList[i].getArrivalBasis()]);
            x.push_back(relativePortal.getX() + guyArrivalList[i].getX());
            y.push_back(relativePortal.getY() + guyArrivalList[i].getY());
            xspeed.push_back(0);
			yspeed.push_back(guyArrivalList[i].getYspeed() + relativePortal.getYspeed() + env.gravity);
        }
        supported.push_back(false);
        supportedSpeed.push_back(0);
        finishedWith.push_back(false);
        facing.push_back(guyArrivalList[i].getFacing());

        // Check with triggers if guy should affect frame
		if (not triggerFrameState.shouldArrive(guyArrivalList[i]))
		{
			finishedWith[i] = true;
			continue;
		}

		// Collision algo
        if (guyArrivalList[i].getIndex() < playerInput.size())
        {
            std::size_t relativeIndex(guyArrivalList[i].getIndex());
            InputList const& input(playerInput[relativeIndex]);

            int const width(guyArrivalList[i].getWidth());
            int const height(guyArrivalList[i].getHeight());

            // chonofrag with platforms
			foreach (Collision const& platform, nextPlatform)
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
					finishedWith[i] = true;
					continue;
				}
			}

			// chonofrag with walls
			if (wallAtExclusive(env, x[i], y[i], width, height))
			{
				finishedWith[i] = true;
				continue;
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
            	if (nextBoxNormalDeparture[j])
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
			}

            // check platform collision in Y direction
            foreach (Collision const& platform, nextPlatform)
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
            	finishedWith[i] = true;
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
            foreach (Collision const& platform, nextPlatform)
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
				finishedWith[i] = true;
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
    assert(boost::distance(finishedWith) == boost::distance(guyArrivalList));
    assert(boost::distance(facing) == boost::distance(guyArrivalList));

    mt::std::vector<char>::type carry(guyArrivalList.size());
    mt::std::vector<int>::type carrySize(guyArrivalList.size());
    mt::std::vector<TimeDirection>::type carryDirection(guyArrivalList.size());

    // box carrying
    for (std::size_t i(0), isize(boost::distance(guyArrivalList)); i < isize; ++i)
    {
        if (guyArrivalList[i].getIndex() < playerInput.size() && !finishedWith[i])
        {
        	carry[i] = guyArrivalList[i].getBoxCarrying();
			carrySize[i] = 0;
			carryDirection[i] = hg::INVALID;

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
                        	makeBoxAndTimeWithPortalsAndMutators(
                                nextBox,
                                nextBoxNormalDeparture,
                                nextPortal,
                                mutators,
                                dropX,
                                dropY,
                                0,
                                yspeed[i],
                                dropSize,
                                -1,
                                guyArrivalList[i].getBoxCarryDirection(),
                                triggerFrameState,
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
                    mt::std::vector<ObjectAndTime<Box> >::type::iterator nextBoxIt(nextBox.begin()),nextBoxEnd(nextBox.end());
					mt::std::vector<char>::type::iterator nextBoxNormalDepartureIt(nextBoxNormalDeparture.begin());
                    for (;nextBoxIt != nextBoxEnd; ++nextBoxIt, ++nextBoxNormalDepartureIt)
                    {
                    	if (*nextBoxNormalDepartureIt)
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
								nextBoxNormalDepartureIt = nextBoxNormalDeparture.erase(nextBoxNormalDepartureIt);
								nextBoxEnd = nextBox.end();
								break;
							}
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

    mt::std::vector<int>::type newWidth(guyArrivalList.size());
    mt::std::vector<int>::type newHeight(guyArrivalList.size());
    mt::std::vector<mt::std::map<Ability, int>::type >::type newPickups(guyArrivalList.size());
    mt::std::vector<int>::type illegalPortal(guyArrivalList.size());
    // arrivalBasis is always -1 for normalDeparture

    // time travel, mutator and portal collision, item use
    for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
    {
    	if (finishedWith[i])
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

            int arrivalBasis = -1;
            illegalPortal[i] = -1;

            newWidth[i] = guyArrivalList[i].getWidth();
            newHeight[i] = guyArrivalList[i].getHeight();
            newPickups[i] = mt::std::map<Ability, int>::type(guyArrivalList[i].getPickups());

            TimeDirection nextTimeDirection = guyArrivalList[i].getTimeDirection();
            Frame* nextTime(nextFrame(time, nextTimeDirection));
            assert(time);

            // Mutators
            // Mutator and falling must occur first due to exact frame effects.
            mt::std::vector<int>::type mutatorCollisions;
			for (unsigned j = 0; j < mutators.size(); ++j)
			{
				if (IntersectingRectanglesInclusiveCollisionOverlap(
						x[i], y[i], newWidth[i], newHeight[i],
						mutators[j].getX(), mutators[j].getY(),
						mutators[j].getWidth(), mutators[j].getHeight(),
						mutators[j].getCollisionOverlap()))
				{
					mutatorCollisions.push_back(i);
				}
			}

			if (mutatorCollisions.size() != 0)
			{
				boost::optional<Guy> newGuy = triggerFrameState.mutateObject(
					mutatorCollisions,
					Guy(relativeIndex,
                        x[i], y[i],
						xspeed[i], yspeed[i],
						newWidth[i], newHeight[i],
						guyArrivalList[i].getIllegalPortal(),
						-1,
						supported[i],
						supportedSpeed[i],
						newPickups[i],
						facing[i],
						carry[i],
						carrySize[i],
						carryDirection[i],
						nextTimeDirection
					));
				if (!newGuy)
				{
					finishedWith[i] = true;
					continue;
				}
				x[i] = newGuy->getX();
				y[i] = newGuy->getY();
				xspeed[i] = newGuy->getXspeed();
				yspeed[i] = newGuy->getYspeed();
				newWidth[i] = newGuy->getWidth();
				newHeight[i] = newGuy->getHeight();
				illegalPortal[i] = newGuy->getIllegalPortal();
				// arrivalBasis is missing with good reason
				supported[i] = newGuy->getSupported();
				supportedSpeed[i] = newGuy->getSupportedSpeed();
				newPickups[i] = newGuy->getPickups();
				facing[i] = newGuy->getFacing();
				carry[i] = newGuy->getBoxCarrying();
				carrySize[i] = newGuy->getBoxCarrySize();
				carryDirection[i] = newGuy->getBoxCarryDirection();
				nextTimeDirection = newGuy->getTimeDirection();
				// relativeIndex is missing for obvious reasons
			}

			// Things that do time travel
			// The occurance of one thing precludes subsequent ones
            bool normalDeparture = true;

            // falling through portals
			if (normalDeparture)
			{
				for (unsigned int j = 0; j < nextPortal.size(); ++j)
				{
					if (IntersectingRectanglesInclusiveCollisionOverlap(
							x[i], y[i], newWidth[i], newHeight[i],
							nextPortal[j].getX(), nextPortal[j].getY(),
							nextPortal[j].getWidth(), nextPortal[j].getHeight(),
							nextPortal[j].getCollisionOverlap())
						&& nextPortal[j].getFallable())
					{
						if (guyArrivalList[i].getIllegalPortal() != -1 && nextPortal[j].getIndex() == guyArrivalList[i].getIllegalPortal())
						{
							illegalPortal[i] = j;
						}
						else if (triggerFrameState.shouldPort(j,
								Guy(relativeIndex,x[i], y[i],xspeed[i], yspeed[i], newWidth[i], newHeight[i],
								illegalPortal[i],-1,
								supported[i],supportedSpeed[i], newPickups[i], facing[i],
								carry[i],carrySize[i], carryDirection[i],nextTimeDirection),false))
						{
							if (nextPortal[j].getWinner())
							{
								winFrame = true;
								nextTime = 0;
								normalDeparture = false;
								break;
							}
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
							illegalPortal[i] = nextPortal[j].getIllegalDestination();
							arrivalBasis = nextPortal[j].getDestinationIndex();
							x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination() - nextPortal[j].getXspeed();
							y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination() - nextPortal[j].getYspeed();
							break;
						}
					}
				}
			}

			// for single frame effects
			// "forced" departures occur before those due to input
			if (normalDeparture)
			{
				mt::std::map<Ability, int>::type::iterator timeJump(newPickups[i].find(TIME_JUMP));
				mt::std::map<Ability, int>::type::iterator timeReverse(newPickups[i].find(TIME_REVERSE));

				if (input.getAbility() == hg::TIME_JUMP && timeJump != newPickups[i].end() && timeJump->second != 0)
				{
					nextTime = getArbitraryFrame(getUniverse(time), input.getFrameIdParam(0).getFrameNumber());
					normalDeparture = false;
					if (timeJump->second > 0)
					{
						newPickups[i][hg::TIME_JUMP] = timeJump->second - 1;
					}
				}
				else if (input.getAbility() == hg::TIME_REVERSE && timeReverse != newPickups[i].end() && timeReverse->second != 0)
				{
					normalDeparture = false;
					nextTimeDirection *= -1;
					nextTime = nextFrame(time, nextTimeDirection);
					carryDirection[i] *= -1;
					if (timeReverse->second > 0)
					{
						newPickups[i][hg::TIME_REVERSE] = timeReverse->second - 1;
					}
				}
				else if (input.getUse() == true)
				{
					for (unsigned int j = 0; j < nextPortal.size(); ++j)
					{
						if (IntersectingRectanglesInclusiveCollisionOverlap(
								x[i], y[i], newWidth[i], newHeight[i],
								nextPortal[j].getX(), nextPortal[j].getY(),
								nextPortal[j].getWidth(), nextPortal[j].getHeight(),
								nextPortal[j].getCollisionOverlap())
							&& (triggerFrameState.shouldPort(j,
									Guy(relativeIndex,x[i], y[i],xspeed[i], yspeed[i], newWidth[i], newHeight[i],
									illegalPortal[i],-1,
									supported[i],supportedSpeed[i], newPickups[i], facing[i],
									carry[i],carrySize[i], carryDirection[i],nextTimeDirection),true)))
						{
							if (nextPortal[j].getWinner())
							{
								winFrame = true;
								nextTime = 0;
								normalDeparture = false;
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
							illegalPortal[i] = nextPortal[j].getIllegalDestination();
							arrivalBasis = nextPortal[j].getDestinationIndex();
							x[i] = x[i] - nextPortal[j].getX() + nextPortal[j].getXdestination() - nextPortal[j].getXspeed();
							y[i] = y[i] - nextPortal[j].getY() + nextPortal[j].getYdestination() - nextPortal[j].getYspeed();
							break;
						}
					}
				}
			}

            if (playerInput.size() - 1 == relativeIndex)
            {
                currentPlayerFrame = true;
                //cout << "nextPlayerFrame set to: " << nextPlayerFrame.frame() << "  " << x[i] << "\n";
            }

            if (not normalDeparture)
            {
            	nextGuy.push_back(
					ObjectAndTime<Guy>(
						Guy(
                            relativeIndex + 1,
							x[i], y[i],
							xspeed[i], yspeed[i],
							newWidth[i], newHeight[i],

							illegalPortal[i],
							arrivalBasis,
							supported[i],
							supportedSpeed[i],

							newPickups[i],
							facing[i],

							carry[i],
							carrySize[i],
							carryDirection[i],

							nextTimeDirection
						),
						nextTime
					)
				);
            	finishedWith[i] = true;
            }
        }
        else
        {
            assert(guyArrivalList[i].getIndex() == playerInput.size());
            nextPlayerFrame = true;
            finishedWith[i] = true;
        }
    }

    // Only normal departures make it this far, time for inter-guy communication (eg guns)
    // This can now occur because NO POSITION CHANGE IS POSSIBLE PAST THIS POINT
	for (std::size_t i(0), size(guyArrivalList.size()); i != size; ++i)
	{
		if (finishedWith[i])
		{
			continue;
		}
		nextGuy.push_back(
			ObjectAndTime<Guy>(
				Guy(
					guyArrivalList[i].getIndex() + 1,
					x[i], y[i],
					xspeed[i], yspeed[i],
					newWidth[i], newHeight[i],

					illegalPortal[i],
					-1,
					supported[i],
					supportedSpeed[i],

					newPickups[i],
					facing[i],

					carry[i],
					carrySize[i],
					carryDirection[i],

					 guyArrivalList[i].getTimeDirection()
				),
				nextFrame(time, guyArrivalList[i].getTimeDirection())
			)
		);
	}
}

template <
    typename RandomAccessPortalRange,
    typename RandomAccessMutatorRange>
void makeBoxAndTimeWithPortalsAndMutators(
		mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
		mt::std::vector<char>::type& nextBoxNormalDeparture,
		const RandomAccessPortalRange& portals,
		const RandomAccessMutatorRange& mutators,
		int x,
		int y,
		int xspeed,
		int yspeed,
		int size,
		int oldIllegalPortal,
		TimeDirection oldTimeDirection,
		TriggerFrameState& triggerFrameState,
		Frame* time)
{
	int arrivalBasis = -1;
	int illegalPortal = -1;
	Frame* nextTime(nextFrame(time, oldTimeDirection));


	// Mutator check
	mt::std::vector<int>::type mutatorCollisions;

	for (unsigned i = 0; i < mutators.size(); ++i)
	{
		if (IntersectingRectanglesInclusiveCollisionOverlap(x, y, size, size,
					mutators[i].getX(), mutators[i].getY(),
					mutators[i].getWidth(), mutators[i].getHeight(),
					mutators[i].getCollisionOverlap()))
		{
			mutatorCollisions.push_back(i);
		}
	}

	// send vector of collisions that occured (if any)
	if (mutatorCollisions.size() != 0)
	{
		boost::optional<Box> newBox = triggerFrameState.mutateObject(
			mutatorCollisions,
			Box(
				x,
				y,
				xspeed,
				yspeed,
				size,
				oldIllegalPortal,
				-1,
				oldTimeDirection));
		if (!newBox)
		{
			return; // box was destroyed, do not add
		}
		x = newBox->getX();
		y = newBox->getY();
		xspeed = newBox->getXspeed();
		yspeed = newBox->getYspeed();
		size = newBox->getSize();
		illegalPortal = newBox->getIllegalPortal();
		// arrivalBasis is not settable
		oldTimeDirection = newBox->getTimeDirection();
	}

	bool normalDeparture = true;

	// fall through portals
    for (unsigned i = 0; i < portals.size(); ++i)
    {
        if (IntersectingRectanglesInclusiveCollisionOverlap(x, y, size, size,
        		portals[i].getX(), portals[i].getY(),
        		portals[i].getWidth(), portals[i].getHeight(),
        		portals[i].getCollisionOverlap())
        		&& portals[i].getFallable())
        {
            if (oldIllegalPortal != -1 && portals[i].getIndex() == oldIllegalPortal)
            {
                illegalPortal = i;
            }
            else if (triggerFrameState.shouldPort(i, Box(x,y,xspeed,yspeed,size,oldIllegalPortal,-1,oldTimeDirection), false))
            {
                Frame* portalTime(
                		portals[i].getRelativeTime() ?
                    getArbitraryFrame(getUniverse(time), getFrameNumber(time) + portals[i].getTimeDestination()):
                    getArbitraryFrame(getUniverse(time), portals[i].getTimeDestination()));
                nextTime = portalTime ? nextFrame(portalTime, oldTimeDirection) : 0;
                illegalPortal = portals[i].getIllegalDestination();
                arrivalBasis = portals[i].getDestinationIndex();
                x = x - portals[i].getX() + portals[i].getXdestination();
                y = y - portals[i].getY() + portals[i].getYdestination();
                xspeed = xspeed - portals[i].getXspeed();
                yspeed = yspeed - portals[i].getXspeed();
                normalDeparture = false;
                break;
            }
        }
    }

    nextBoxNormalDeparture.push_back(normalDeparture);

    // add box
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
				arrivalBasis,
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
    std::size_t index,
    int subtractionNumber) // horizontal wins a tie
{
	boxesSoFar.push_back(index);

	for (std::size_t i(0), isize(majorAxis.size()); i < isize; ++i)
	{
		if (i != index && !squished[i] &&
			IntersectingRectanglesExclusive(
                majorAxis[index], minorAxis[index], size[index], size[index],
                majorAxis[i], minorAxis[i], size[i], size[i]) &&
			std::abs(majorAxis[index] - majorAxis[i]) > std::abs(minorAxis[index] - minorAxis[i]) - subtractionNumber)
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

			recursiveBoxCollision(majorAxis, minorAxis, size, squished, boxesSoFar, i, subtractionNumber);
		}
	}
}

template <
    typename RandomAccessBoxRange,
    typename RandomAccessPortalRange,
    typename RandomAccessPlatformRange,
    typename RandomAccessArrivalLocationRange,
    typename RandomAccessMutatorRange>
void boxCollisionAlogorithm(
    Environment const& env,
    RandomAccessBoxRange const& boxArrivalList,
    mt::std::vector<Box>::type const& additionalBox,
    mt::std::vector<ObjectAndTime<Box> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    RandomAccessPlatformRange const& nextPlatform,
    RandomAccessPortalRange const& nextPortal,
    RandomAccessArrivalLocationRange const& arrivalLocations,
    RandomAccessMutatorRange const& mutators,
    TriggerFrameState& triggerFrameState,
    Frame* time)
{
	mt::std::vector<Box>::type oldBoxList;

	boost::push_back(oldBoxList, boxArrivalList);
	boost::push_back(oldBoxList, additionalBox);

	boost::sort(oldBoxList);

	mt::std::vector<int>::type x(oldBoxList.size());
	mt::std::vector<int>::type y(oldBoxList.size());
	mt::std::vector<int>::type xTemp(oldBoxList.size());
	mt::std::vector<int>::type yTemp(oldBoxList.size());
	mt::std::vector<int>::type size(oldBoxList.size());
	mt::std::vector<char>::type squished(oldBoxList.size());

	// Check with triggers if the box should arrive at all
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		if (not triggerFrameState.shouldArrive(oldBoxList[i]))
		{
			squished[i] = true;
		}
	}

	// Inititalise box location with arrival basis
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
        if (oldBoxList[i].getArrivalBasis() == -1)
        {
            xTemp[i] = oldBoxList[i].getX();
            yTemp[i] = oldBoxList[i].getY();
            x[i] = xTemp[i] + oldBoxList[i].getXspeed();
			y[i] = yTemp[i] + oldBoxList[i].getYspeed() + env.gravity;
        }
        else
        {
            ArrivalLocation const& relativePortal(arrivalLocations[oldBoxList[i].getArrivalBasis()]);
            xTemp[i] = relativePortal.getX() + oldBoxList[i].getX();
            yTemp[i] = relativePortal.getY() + oldBoxList[i].getY();
            x[i] = xTemp[i] + oldBoxList[i].getXspeed() + relativePortal.getXspeed();
			y[i] = yTemp[i] + oldBoxList[i].getYspeed() + relativePortal.getYspeed() + env.gravity;
        }
        size[i] = oldBoxList[i].getSize();
	}

	// Destroy boxes that are overlapping with platforms and walls
	for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
	{
		if (not squished[i])
		{
			if (env.wall.at(xTemp[i], yTemp[i]) && env.wall.at(xTemp[i]+size[i]-1, yTemp[i])
					&& env.wall.at(xTemp[i], yTemp[i]+size[i]-1) && env.wall.at(xTemp[i]+size[i], yTemp[i]+size[i]-1))
			{
				squished[i] = true;
				continue;
			}

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
	bool thereAreStillThingsToDo(true); // if a box moves thereAreStillThingsToDo
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

		//*** collide boxes with platforms and walls to discover the hard bounds on the system ***//
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
				//** Check inside a wall, velocity independent which is why it is so complex **//

				// intial keep-it-inside-the-level step
				if (x[i] <= 0 || y[i] <= 0 || x[i] + size[i] > env.wall.roomWidth() || y[i] + size[i] > env.wall.roomHeight())
				{
					int xOut = x[i] + size[i] - env.wall.roomWidth();
					int yOut = y[i] + size[i] - env.wall.roomHeight();
					if (x[i] < 0 && (y[i] > 0 || x[i] < y[i]) && (yOut <= 0 || -x[i] > yOut))
					{
						y[i] = y[i] - (x[i]-env.wall.segmentSize()/2)*(y[i]-yTemp[i])/std::abs(x[i]-xTemp[i]);
						x[i] = env.wall.segmentSize()/2;
					}
					else if (y[i] < 0 && (x[i] > 0 || y[i] < x[i]) && (xOut <= 0 || -y[i] > xOut))
					{
						x[i] = x[i] - (y[i]-env.wall.segmentSize()/2)*(x[i]-xTemp[i])/std::abs(y[i]-yTemp[i]);
						y[i] = env.wall.segmentSize()/2;
					}
					else if (xOut > 0 && (y[i] > 0 || xOut > -y[i]) && (yOut <= 0 || xOut > yOut))
					{
						y[i] = y[i] - (xOut+env.wall.segmentSize()/2+size[i])*(y[i]-yTemp[i])/std::abs(x[i]-xTemp[i]);
						x[i] = env.wall.roomWidth() - env.wall.segmentSize()/2 - size[i];
					}
					else if (yOut > 0 && (x[i] > 0 || yOut > -x[i]) && (xOut <= 0 || yOut > xOut))
					{
						x[i] = x[i] - (yOut+env.wall.segmentSize()/2+size[i])*(x[i]-xTemp[i])/std::abs(y[i]-yTemp[i]);
						y[i] = env.wall.roomHeight() - env.wall.segmentSize()/2 - size[i];
					}
				}

				// TryAgainWithMoreInterpolation is only to be triggered only when moving
				// the box by size[i] will ensure that the box still collides with the wall
				// it is attempted to be moved out of.
				if (false)
				{
					TryAgainWithMoreInterpolation:;
					if (std::abs(x[i]-xTemp[i]) < std::abs(y[i]-yTemp[i]))
					{
						x[i] = x[i] - env.wall.segmentSize()*(x[i]-xTemp[i])/std::abs(y[i]-yTemp[i]);
						y[i] = y[i] - env.wall.segmentSize()*(y[i]-yTemp[i])/std::abs(y[i]-yTemp[i]);
					}
					else
					{
						y[i] = y[i] - env.wall.segmentSize()*(y[i]-yTemp[i])/std::abs(x[i]-xTemp[i]);
						x[i] = x[i] - env.wall.segmentSize()*(x[i]-xTemp[i])/std::abs(x[i]-xTemp[i]);
					}
				}

				top[i] = std::make_pair(false, 0);
				bottom[i] = std::make_pair(false, 0);
				left[i] = std::make_pair(false, 0);
				right[i] = std::make_pair(false, 0);

				// Normal case; box does not have 2 or more sides lined up on the wallmap.
				bool w00,w10,w01,w11;

				if (size[i] <= env.wall.segmentSize())
				{
					// purely a speedup for this case
					w00 = env.wall.at(x[i], y[i]);
					w10 = env.wall.at(x[i]+size[i]-1, y[i]);
					w01 = env.wall.at(x[i], y[i]+size[i]-1);
					w11 = env.wall.at(x[i]+size[i]-1, y[i]+size[i]-1);
				}
				else
				{
					// Extra collision for box size greater than wall size (would handle other case too)
					w00 = false;
					w10 = false;
					w01 = false;
					w11 = false;

					int xOff = 0;
					while (xOff < size[i]-1)
					{
						xOff += env.wall.segmentSize();
						if (xOff > size[i]-1)
						{
							xOff = size[i]-1;
						}
						int yOff = 0;
						while (yOff < size[i]-1)
						{
							yOff += env.wall.segmentSize();
							if (yOff > size[i]-1)
							{
								yOff = size[i]-1;
							}
							// the collision test bit
							if (!w00)
							{
								w00 = env.wall.at(x[i]+size[i]-xOff, y[i]+size[i]-yOff);
							}
							if (!w10)
							{
								w10 = env.wall.at(x[i]+yOff, y[i]+size[i]-yOff);
							}
							if (!w01)
							{
								w01 = env.wall.at(x[i]+size[i]-xOff, y[i]+yOff);
							}
							if (!w01)
							{
								w01 = env.wall.at(x[i]+xOff, y[i]+yOff);
							}
						}
					}
				}

				// collide with walls based on corner status
				if (w00)
				{
					if (w11)
					{
						if (w10)
						{
							if (w01)
							{
								goto TryAgainWithMoreInterpolation;
							}
							else
							{
								x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
								right[i] = std::make_pair(true, x[i]);
								y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
								top[i] = std::make_pair(true, y[i]);
							}
						}
						else if (w01 || !env.wall.inTopRightTriangle(x[i],y[i])) // this triangle check needs improvement for rectangles
						{
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = std::make_pair(true, y[i]);
						}
						else
						{
							x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							right[i] = std::make_pair(true, x[i]);
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}

					}
					else if (w10)
					{
						if (w01)
						{
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
						else
						{
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w01 || env.wall.inTopRightTriangle(x[i],y[i]))
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
					}
					else
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
				}
				else if (w10)
				{
					if (w01)
					{
						if (w11 || env.wall.inTopLeftTriangle(x[i]+size[i]-1,y[i])) // this triangle check needs improvement for rectangles
						{
							x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							right[i] = std::make_pair(true, x[i]);
							y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
							bottom[i] = std::make_pair(true, y[i]);
						}
						else
						{
							x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							left[i] = std::make_pair(true, x[i]);
							y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
							top[i] = std::make_pair(true, y[i]);
						}
					}
					else if (w11 || env.wall.inTopLeftTriangle(x[i]+size[i]-1,y[i]))
					{
						x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						right[i] = std::make_pair(true, x[i]);
					}
					else
					{
						y[i] = (y[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						top[i] = std::make_pair(true, y[i]);
					}
				}
				else if (w01)
				{
					if (w11 || env.wall.inTopLeftTriangle(x[i],y[i]+size[i]-1))
					{
						y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = std::make_pair(true, y[i]);
						x[i] = xTemp[i];
					}
					else
					{
						x[i] = (x[i]/env.wall.segmentSize()+1)*env.wall.segmentSize();
						left[i] = std::make_pair(true, x[i]);
					}
				}
				else if (w11)
				{
					if (env.wall.inTopRightTriangle(x[i]+size[i]-1,y[i]+size[i]-1))
					{
						y[i] = ((y[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
						bottom[i] = std::make_pair(true, y[i]);
						x[i] = xTemp[i];
					}
					else
					{
						x[i] = ((x[i]+size[i]-1)/env.wall.segmentSize())*env.wall.segmentSize()-size[i];
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
                        if (std::abs(x[i] - x[j]) >= std::abs(y[i] - y[j])) // left or right
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
				recursiveBoxCollision(y, x, size, squished, pass, i, 0);
			}
		}

		// And now in that other dimension!
		for (std::size_t i(0), isize(boost::distance(oldBoxList)); i < isize; ++i)
		{
			if (!squished[i])
			{
			    mt::std::vector<std::size_t>::type pass;
				recursiveBoxCollision(x, y, size, squished, pass, i, 1);
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
			if (oldBoxList[i].getArrivalBasis() == -1)
			{
				makeBoxAndTimeWithPortalsAndMutators(
                    nextBox,
                    nextBoxNormalDeparture,
                    nextPortal,
                    mutators,
                    x[i],
                    y[i],
                    x[i] - oldBoxList[i].getX(),
                    y[i] - oldBoxList[i].getY(),
                    size[i],
                    oldBoxList[i].getIllegalPortal(),
                    oldBoxList[i].getTimeDirection(),
                    triggerFrameState,
                    time);
			}
			else
			{
				ArrivalLocation const& relativePortal(arrivalLocations[oldBoxList[i].getArrivalBasis()]);
				makeBoxAndTimeWithPortalsAndMutators(
                    nextBox,
                    nextBoxNormalDeparture,
                    nextPortal,
                    mutators,
                    x[i],
                    y[i],
                    x[i] - relativePortal.getX() - oldBoxList[i].getX(),
                    y[i] - relativePortal.getY() - oldBoxList[i].getY(),
                    size[i],
                    oldBoxList[i].getIllegalPortal(),
                    oldBoxList[i].getTimeDirection(),
                    triggerFrameState,
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

#if 0
unsigned int Raytrace(
		Environment const& env,
		int sx, int sy, int px, int py, // start and position of target for trace
		mt::std::vector<ObjectAndTime<Box> >::type box,
		mt::std::vector<char>::type& nextBoxNormalDeparture,
		mt::std::vector<int>::type gx, // other guy things
		mt::std::vector<int>::type gy,
		mt::std::vector<int>::type gw,
		mt::std::vector<int>::type gh,
		mt::std::vector<char>::type shootable,
		int myIndex) // don't hit myself
{
	// starts in a wall all it can hit is a wall
	if (env.wall.at(sx,sy))
	{
		return -1;
	}

	// shoot to the right if the direction has no length
	if (sx == px && sy == py)
	{
		px = sx + 1;
	}

	int size = env.wall.segmentSize();

	if (sx < px && sy >= py) // top right, x safe
	{
		// find wall distance
		int tx = sx;
		int ty = sy;
		while (true)
		{
			int hDis = tx%size + size - sx;
			int vDis = (ty-1)%size - sy;
			if (vDis <= hDis * (py - sy)/(px - sx)) // both sides are non-positive
			{
				// next grid square on Right
				tx = tx%size + size;
				ty = tx * (py - sy)/(px - sx) + sy;
				if (env.wall.at(tx,ty))
				{
					break;
				}
			}
			else
			{
				// next grid square on Top
				ty = (ty-1)%size;
				tx = ty * (px - sx)/(py - sy) + sx; // it is known (py - sy) != 0
				if (env.wall.at(tx,ty))
				{
					break;
				}
			}
		}
		int wallDistance = tx - sx;

		// find all the boxes within wallDistance and on line
		for (std::size_t j(0), jsize(box.size()); j < jsize; ++j)
		{
			// only shoot normal departure boxes
			if (nextBoxNormalDeparture[j])
			{

			}
		}

	}
	else if (sx >= px && sy > py) // top left, y safe
	{

	}
	else if (sx > px && sy <= py) // bottom left, x safe
	{

	}
	else // (sx <= px && sy < py) // bottom right, y safe
	{
		assert(sx <= px && sy < py);

	}
}
#endif

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
	);
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
	);
}


bool IntersectingRectanglesInclusiveCollisionOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int buffer)
{
	if (buffer < HALF_COLLISION_BUFFER_RANGE)
	{
		return IntersectingRectanglesInclusive(
			x1 + w1 * buffer/COLLISION_BUFFER_RANGE,
			y1 + h1 * buffer/COLLISION_BUFFER_RANGE,
			w1 - w1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			h1 - h1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			x2, y2, w2, h2
		);
	}
	else if (buffer == HALF_COLLISION_BUFFER_RANGE)
	{
		return PointInRectangleInclusive(x1 + w1/2, y1 + h1/2, x2, y2, w2, h2);
	}
	else //(buffer > HALF_COLLISION_BUFFER_RANGE)
	{
		// Yes h1 and w1 are negative
		// the intersection algorithms don't care and it's faster not to prevent negativity
		return RectangleWithinInclusive(
			x1 + w1 * buffer/COLLISION_BUFFER_RANGE,
			y1 + h1 * buffer/COLLISION_BUFFER_RANGE,
			w1 - w1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			h1 - h1 * buffer/HALF_COLLISION_BUFFER_RANGE,
			x2, y2, w2, h2
		);
	}
}

bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return
		(x1 >= x2 && x1 + w1 <= x2 + w2)
		&&
		(y1 >= y2 && y1 + h1 <= y2 + h2);
}

} //namespace
} //namespace hg
