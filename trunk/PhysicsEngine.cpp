#include "PhysicsEngine.h"

#include "TimeObjectListList.h"
#include "ObjectList.h"
#include "TimeDirection.h"

#include <iostream>
#include <cassert>
#include <map>
#include <vector>
#include <math.h>

using namespace ::std;
using namespace ::hg;

PhysicsEngine::PhysicsEngine(vector<vector<bool> > newWallmap,
                             int newWallSize,
                             int newGravity,
                             AttachmentMap nAttachmentMap,
                             TriggerSystem nTriggerSystem) :
wallmap(newWallmap),
gravity(newGravity),
wallSize(newWallSize),
attachmentMap(nAttachmentMap),
triggerSystem(nTriggerSystem)
{
}

TimeObjectListList PhysicsEngine::executeFrame(const ObjectList& arrivals,
                                               const NewFrameID time,
                                               const std::vector<InputList>& playerInput,
                                               NewFrameID& currentPlayerFrame,
                                               TimeDirection& currentPlayerDirection,
                                               NewFrameID& nextPlayerFrame) const
{
    std::vector<BoxInfo> nextBox;
    std::vector<Platform> nextPlatform;
    std::vector<PlatformDestination> platformDesinations;
	map<NewFrameID, MutableObjectList> newDepartures;

	// Trigger system execution
    triggerSystem.getPlatformDestinations(arrivals.getButtonListRef(), platformDesinations);

	// platforms set their new location and velocity from trigger system data (and ofc their physical data)
	platformStep(arrivals.getPlatformListRef(), nextPlatform, platformDesinations, newDepartures, time);

	// pickup position update from platform

	// boxes do their crazy wizz-bang collision algorithm
    crappyBoxCollisionAlogorithm(arrivals.getBoxListRef(), nextBox, nextPlatform, newDepartures);

	// item simple collision algorithm

	// guys simple collision algorithm
	guyStep(arrivals.getGuyListRef(), time, playerInput,
            newDepartures, nextBox, nextPlatform, currentPlayerFrame, nextPlayerFrame, currentPlayerDirection);

    // button state and position update
	buttonChecks(arrivals.getBoxListRef(), arrivals.getGuyListRef(), arrivals.getButtonListRef(), nextPlatform, newDepartures, time);

    // build departures for boxes
	for (size_t i = 0; i < nextBox.size(); ++i)
	{
		NewFrameID nextTime(time.nextFrame(nextBox[i].box.getTimeDirection()));

		if (nextTime.isValidFrame())
		{
			newDepartures[nextTime].addBox(nextBox[i].box);
		}
	}

    // compile all departures
    TimeObjectListList returnDepartures;

    for (map<NewFrameID, MutableObjectList>::iterator it(newDepartures.begin()), end(newDepartures.end()); it != end; ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        returnDepartures.insertObjectList(it->first, ObjectList(it->second));
    }
	// add data to departures
	return returnDepartures;
}

void PhysicsEngine::guyStep(const vector<Guy>& oldGuyList,
                            const NewFrameID time,
                            const vector<InputList>& playerInput,
                            map<NewFrameID, MutableObjectList>& newDepartures,
                            std::vector<BoxInfo>& nextBox,
                            const ::std::vector<Platform>& nextPlatform,
                            NewFrameID& currentPlayerFrame,
                            NewFrameID& nextPlayerFrame,
                            TimeDirection& currentPlayerDirection) const
{
	vector<int> x;
	vector<int> y;
	vector<int> xspeed;
	vector<int> yspeed;
	vector<bool> supported;

    x.reserve(oldGuyList.size());
    y.reserve(oldGuyList.size());
    xspeed.reserve(oldGuyList.size());
    yspeed.reserve(oldGuyList.size());

	// position, velocity, collisions
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getRelativeIndex() < playerInput.size())
        {
            x.push_back(oldGuyList[i].getX());
            y.push_back(oldGuyList[i].getY());
            xspeed.push_back(0);
            yspeed.push_back(oldGuyList[i].getYspeed() + gravity);
            supported.push_back(false);

            size_t relativeIndex = oldGuyList[i].getRelativeIndex();
            const InputList& input = playerInput[relativeIndex];

            int width = oldGuyList[i].getWidth();
            int height = oldGuyList[i].getHeight();

             // jump
            if (oldGuyList[i].getSupported() && input.getUp())
            {
                yspeed[i] = -700;
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

                if (intersectingRectangles(x[i], newY, width, height, pX, pY, pWidth, pHeight, false))
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
                int boxX = nextBox[j].box.getX();
                int boxY = nextBox[j].box.getY();
                int boxXspeed = nextBox[j].box.getXspeed();
                int boxYspeed = nextBox[j].box.getYspeed();
                int boxSize = nextBox[j].box.getSize();
                TimeDirection boxDirection = nextBox[j].box.getTimeDirection();
                if (x[i] <= boxX+boxSize && x[i]+width >= boxX)
                {
                    if (boxDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
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

                if (intersectingRectangles(newX, newY, width, height, pX, pY, pWidth, pHeight, false))
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

	vector<bool> carry;
	vector<int> carrySize;
	vector<TimeDirection> carryDirection;

	// box carrying
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getRelativeIndex() < playerInput.size())
        {
            carry.push_back(oldGuyList[i].getBoxCarrying());
            carrySize.push_back(0);
            carryDirection.push_back(hg::INVALID);

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
                            if (intersectingRectangles(nextBox[j].box.getX(), nextBox[j].box.getY(), nextBox[j].box.getSize(), nextBox[j].box.getSize(),
                                                      dropX, dropY, dropSize, dropSize, false))
                            {
                                droppable = false;
                            }
                        }
                        for (unsigned int j = 0; droppable && j < nextPlatform.size(); ++j)
                        {
                            if (intersectingRectangles(nextPlatform[j].getX(), nextPlatform[j].getY(),
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
                                Box(
                                    dropX,
                                    dropY,
                                    0,
                                    0,
                                    dropSize,
                                    oldGuyList[i].getBoxCarryDirection()
                                  ),
                              false
                              )
                             );
                            carry[i] = false;
                            carrySize[i] = 0;
                            carryDirection[i] = hg::INVALID;
                        }
                    }
                }

                if (!droppable)
                {
                    carrySize[i] = oldGuyList[i].getBoxCarrySize();
                    carryDirection[i] = oldGuyList[i].getBoxCarryDirection();
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
                        int boxX = nextBox[j].box.getX();
                        int boxY = nextBox[j].box.getY();
                        int boxSize = nextBox[j].box.getSize();
                        if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY))
                        {
                            carry[i] = true;
                            carrySize[i] = boxSize;
                            carryDirection[i] = nextBox[j].box.
                            getTimeDirection();
                            nextBox.erase(nextBox.begin() + j);
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
	// colliding with pickups?

	// write to departure arrays, other little things
	for (size_t i = 0; i < oldGuyList.size(); ++i)
	{
        if (oldGuyList[i].getRelativeIndex() < playerInput.size()) {
            // animation
            int nextSubimage = oldGuyList[i].getSubimage() + 1;
            if (nextSubimage > Guy::animationLength)
            {
                nextSubimage = 0;
            }

            size_t relativeIndex(oldGuyList[i].getRelativeIndex());

            const InputList& input = playerInput[relativeIndex];

            // add departure for guy at the appropriate time
            TimeDirection nextTimeDirection = oldGuyList[i].getTimeDirection();
            NewFrameID nextTime(time.nextFrame(nextTimeDirection));
            assert(nextTime.isValidFrame());
            assert(time.isValidFrame());
            if (input.getAbility() == hg::TIME_JUMP)
            {
                nextTime = input.getFrameIdParam(0);
            }
            else if (input.getAbility() == hg::TIME_REVERSE)
            {
                nextTimeDirection *= -1;
                nextTime = time.nextFrame(nextTimeDirection);
                carryDirection[i] *= -1;
            }

            if (playerInput.size() - 1 == relativeIndex)
            {
                currentPlayerDirection = oldGuyList[i].getTimeDirection();
                currentPlayerFrame = time;
                nextPlayerFrame = nextTime;
                assert(nextPlayerFrame.isValidFrame());
            }

            if (nextTime.isValidFrame())
            {
                newDepartures[nextTime].addGuy
                (
                    Guy(x[i], y[i], xspeed[i], yspeed[i],
                        oldGuyList[i].getWidth(), oldGuyList[i].getHeight(),
                        supported[i],
                        carry[i], carrySize[i], carryDirection[i],
                        nextTimeDirection,
                        relativeIndex+1, nextSubimage)
                );
            }
        }
        else
        {
            assert(oldGuyList[i].getRelativeIndex() == playerInput.size());
        }
    }
}

void PhysicsEngine::crappyBoxCollisionAlogorithm(const vector<Box>& oldBoxList,
                                                 std::vector<BoxInfo>& nextBox,
                                                 const ::std::vector<Platform>& nextPlatform,
                                                 const ::std::map<NewFrameID, MutableObjectList>& newDepartures) const
{
	for (vector<Box>::const_iterator i(oldBoxList.begin()), iend(oldBoxList.end()); i != iend; ++i)
	{
		int x = i->getX();
		int y = i->getY();
		int xspeed = i->getXspeed();
		int yspeed = i->getYspeed() + gravity;
		int size = i->getSize();

		bool supported = false;

        bool exploded = false;

        for (vector<Box>::const_iterator j(oldBoxList.begin()), jend(oldBoxList.end()); j != jend; ++j)
        {
            if (j != i)
            {
                int boxX = j->getX();
                int boxY = j->getY();
                int boxSize = j->getSize();
                if (PhysicsEngine::intersectingRectangles(x, y, size, size, boxX, boxY, boxSize, boxSize, false))
                {
                    exploded = true;
                    continue;
                }
            }
        }
        /*
        for (vector<Platform>::const_iterator j(nextPlatform.begin()), jend(nextPlatform.end()); j != jend; ++j)
        {
            if (PhysicsEngine::intersectingRectangles(x, y, size, size, j->getX(), j->getY(), j->getHeight(), j->getWidth(), false))
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
                if (PhysicsEngine::intersectingRectangles(x, newY, size, size, boxX, boxY, boxSize, boxSize, true))
                {
                    if (newY + size/2 < boxY + boxSize/2)
                    {
                        newY = boxY - size;
                    }
                    else
                    {
                        newY = boxY + boxSize;
                    }
                }
            }
        }
        yspeed = newY - y;

		//check wall collision in Y direction
		if (yspeed > 0) // down
		{
			if (wallmap[x/wallSize][(newY+size)/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][(newY+size)/wallSize]))
			{
				newY = ((newY+size)/wallSize)*wallSize - size;
				supported = true;
				xspeed = 0;
			}
		}
		else if (yspeed < 0) // up
		{
			if  (wallmap[x/wallSize][newY/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][newY/wallSize]))
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
            if (pDirection*i->getTimeDirection() == hg::REVERSE)
            {
                pX -= nextPlatform[j].getXspeed();
                pY -= nextPlatform[j].getYspeed();
            }
            int pWidth = nextPlatform[j].getWidth();
            int pHeight = nextPlatform[j].getHeight();

            if (intersectingRectangles(x, newY, size, size, pX, pY, pWidth, pHeight, false))
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
                    if (PhysicsEngine::intersectingRectangles(newX, newY, size, size, boxX, boxY, boxSize, boxSize, true))
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

            if (intersectingRectangles(newX, newY, size, size, pX, pY, pWidth, pHeight, false))
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
                    i->getTimeDirection()
                )
                ,supported
            )
        );

	}
}

void PhysicsEngine::platformStep(const ::std::vector<Platform>& oldPlatformList,
                                 ::std::vector<Platform>& nextPlatform,
                                 const std::vector<PlatformDestination>& pd,
                                 ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                                 const NewFrameID& time) const
{

    for (int i = 0; i < oldPlatformList.size(); ++i)
    {
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
                    if (abs(x - pd[i].getX()) > ((int)pow((double)xspeed - direction*pd[i].getXaccel(),2))*3/(2*pd[i].getXdeccel()))
                    {
                        // fully accelerate
                        xspeed -= direction*pd[i].getXaccel();
                    }
                    // if the platform can stop if it doesn't accelerate
                    else if (abs(x - pd[i].getX()) > ((int)pow((double)xspeed,2))*3/(2*pd[i].getXdeccel()))
                    {
                        // set speed to required speed
                        xspeed = -direction*floor(sqrt((double)abs(x - pd[i].getX())*pd[i].getXdeccel()*2/3));
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
                        yspeed = -direction*floor(sqrt((double)abs(y - pd[i].getY())*pd[i].getYdeccel()*2/3));
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
        nextPlatform.push_back(Platform(x, y, xspeed, yspeed, oldPlatformList[i].getWidth(), oldPlatformList[i].getHeight(), oldPlatformList[i].getIndex(), oldPlatformList[i].getTimeDirection()));

        NewFrameID nextTime(time.nextFrame(oldPlatformList[i].getTimeDirection()));

        if (nextTime.isValidFrame())
        {
            newDepartures[nextTime].addPlatform
            (
                Platform(x, y, xspeed, yspeed, oldPlatformList[i].getWidth(), oldPlatformList[i].getHeight(), oldPlatformList[i].getIndex(), oldPlatformList[i].getTimeDirection())
            );
        }
    }
}

void PhysicsEngine::buttonChecks(const ::std::vector<Box>& oldBoxList,
                                 const ::std::vector<Guy>& oldGuyList,
                                 const ::std::vector<Button>& oldButtonList,
                                 const ::std::vector<Platform>& nextPlatform,
                                 ::std::map<NewFrameID, MutableObjectList>& newDepartures,
                                 NewFrameID time) const
{
    ::std::vector< ::boost::tuple<int, int, int> > attachments = attachmentMap.getButtonAttachmentRef();

    for (unsigned int i = 0; i < oldButtonList.size(); ++i)
	{
	    int x = oldButtonList[i].getX();
	    int y = oldButtonList[i].getY();
	    int w = 3200;
	    int h = 800;

	    if (attachments[i].get<0>() != -1)
	    {
	        int pid = attachments[i].get<0>();
/*
	        if (nextPlatform[pid].getTimeDirection()*oldButtonList[i].getTimeDirection() == hg::FORWARDS)
	        {
	            x = nextPlatform[pid].getX()+attachments[i].get<1>();
                y = nextPlatform[pid].getY()+attachments[i].get<2>();
	        }
	        else
	        {
	            x = nextPlatform[pid].getX()-nextPlatform[pid].getXspeed()+attachments[i].get<1>();
                y = nextPlatform[pid].getY()-nextPlatform[pid].getYspeed()+attachments[i].get<2>();
	        }*/
	    }

	    bool state = false;

	    for (unsigned int j = 0; !state && j < oldBoxList.size(); ++j)
        {
            state = PhysicsEngine::intersectingRectangles(x, y, w, h, oldBoxList[j].getX(), oldBoxList[j].getY(), oldBoxList[j].getSize(), oldBoxList[j].getSize(), true);
        }

         for (unsigned int j = 0; !state && j < oldGuyList.size(); ++j)
        {
            state = PhysicsEngine::intersectingRectangles(x, y, w, h, oldGuyList[j].getX(), oldGuyList[j].getY(), oldGuyList[j].getWidth(), oldGuyList[j].getHeight(), true);
        }

        NewFrameID nextTime(time.nextFrame(oldButtonList[i].getTimeDirection()));

        if (nextTime.isValidFrame())
        {
            newDepartures[nextTime].addButton
            (
                Button(x, y, x-oldButtonList[i].getX(), y-oldButtonList[i].getY(),0, state, oldButtonList[i].getTimeDirection())
            );
        }

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

bool PhysicsEngine::intersectingRectangles(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, bool inclusive) const
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
