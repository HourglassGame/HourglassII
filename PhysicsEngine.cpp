#include "PhysicsEngine.h"

#include "TimeObjectListList.h"
#include "ObjectList.h"
#include "TimeDirection.h"

#include <iostream>
#include <cassert>
#include <map>
#include <vector>

using namespace ::std;
using namespace ::hg;

PhysicsEngine::PhysicsEngine(unsigned int newTimeLineLength, vector<vector<bool> > newWallmap, int newWallSize, int newGravity) :
timeLineLength(newTimeLineLength),
wallmap(newWallmap),
gravity(newGravity),
wallSize(newWallSize)
{
}

TimeObjectListList PhysicsEngine::executeFrame(const ObjectList& arrivals,
                                               const FrameID time,
                                               const std::vector<InputList>& playerInput,
                                               FrameID& currentPlayerFrame,
                                               TimeDirection& currentPlayerDirection,
                                               FrameID& nextPlayerFrame) const
{
    std::vector<BoxInfo> nextBox;

	map<FrameID, MutableObjectList> newDepartures;

	// Switch Collisions at this point?

	// Trigger system execution

	// platforms set their new location and velocity from trigger system data (and ofc their physical data)

	// switch position update
	// pickup position update
	crappyBoxCollisionAlogorithm(arrivals.getBoxListRef(), nextBox);
	// boxes do their crazy wizz-bang collision algorithm

	// item simple collision algorithm

	// guys simple collision algorithm
	guyStep(arrivals.getGuyListRef(), time, playerInput,
            newDepartures, nextBox, currentPlayerFrame, nextPlayerFrame, currentPlayerDirection);
	// guys pickup pickups

	// guys pickup/put down boxes and objects

	// guys do timetravel-type stuff
	for (size_t i = 0; i < nextBox.size(); ++i)
	{
		FrameID nextTime(time+nextBox[i].box.getTimeDirection());

		if (nextTime >= 0 && nextTime < timeLineLength)
		{
			newDepartures[nextTime].addBox(nextBox[i].box);
		}
	}

    TimeObjectListList returnDepartures;

    for (map<FrameID, MutableObjectList>::iterator it(newDepartures.begin()), end(newDepartures.end()); it != end; ++it) {
        //Consider adding insertObjectList overload which can take aim for massively increased efficiency
        returnDepartures.insertObjectList(it->first, ObjectList(it->second));
    }
	// add data to departures
	return returnDepartures;
}

void PhysicsEngine::guyStep(const vector<Guy>& oldGuyList,
                            const FrameID time,
                            const vector<InputList>& playerInput,
                            map<FrameID, MutableObjectList>& newDepartures,
                            std::vector<BoxInfo>& nextBox,
                            FrameID& currentPlayerFrame,
                            FrameID& nextPlayerFrame,
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

            unsigned int relativeIndex = oldGuyList[i].getRelativeIndex();
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

            // box collision
            for (unsigned int j = 0; j < nextBox.size(); ++j)
            {
                int boxX = nextBox[j].box.getX();
                int boxY = nextBox[j].box.getY();
                int boxXspeed = nextBox[j].box.getXspeed();
                int boxYspeed = nextBox[j].box.getYspeed();
                int boxSize = nextBox[j].box.getSize();
                TimeDirection boxDirection = nextBox[j].box.getTimeDirection();
                if (x[i] <= boxX+boxSize and x[i]+width >= boxX)
                {
                    if (boxDirection*oldGuyList[i].getTimeDirection() == hg::REVERSE)
                    {
                        if (newY+height >= boxY-boxYspeed and newY+height-yspeed[i] <= boxY)
                        {
                            newY = boxY-height-boxYspeed;
                            xspeed[i] = -boxXspeed;
                            supported[i] = true;
                        }

                    }
                    else
                    {
                        if (newY+height >= boxY and newY-yspeed[i]+height <= boxY-boxYspeed)
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
            carryDirection.push_back(hg::PAUSE);

            unsigned int relativeIndex = oldGuyList[i].getRelativeIndex();
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
                        for (unsigned int j = 0; j < nextBox.size(); ++j)
                        {
                            if (intersectingRectangles(nextBox[i].box.getX(), nextBox[i].box.getY(), nextBox[i].box.getSize(), nextBox[i].box.getSize(),
                                                      dropX, dropY, dropSize, dropSize, false))
                            {
                                cout << "not droppable" << endl;
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
                            carryDirection[i] = hg::PAUSE;
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
                    carryDirection[i] = hg::PAUSE;
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
            FrameID nextTime(time+nextTimeDirection);

            if (input.getAbility() == hg::TIME_JUMP)
            {
                nextTime = input.getFrameIdParam(0);
                //cout << time << endl << nextTime << endl << endl;
            }
            else if (input.getAbility() == hg::TIME_REVERSE)
            {
                nextTimeDirection *= -1;
                nextTime = time+nextTimeDirection;
                carryDirection[i] *= -1;
            }

            if (playerInput.size() - 1 == relativeIndex)
            {
                currentPlayerDirection = oldGuyList[i].getTimeDirection();
                currentPlayerFrame = time;
                nextPlayerFrame = nextTime;
            }

            if (nextTime >= 0 && nextTime < timeLineLength)
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
                                                 std::vector<BoxInfo>& nextBox) const
{
	for (unsigned int i = 0; i < oldBoxList.size(); ++i)
	{
		int x = oldBoxList[i].getX();
		int y = oldBoxList[i].getY();
		int xspeed = oldBoxList[i].getXspeed();
		int yspeed = oldBoxList[i].getYspeed() + gravity;
		int size = oldBoxList[i].getSize();

		bool supported = false;

        // ** Y component **

        int newY = y + yspeed;

		// box collision

		int yComponent = 0;
		vector<int> colliders = vector<int>();
        for (unsigned int j = 0; j < oldBoxList.size(); ++j)
        {
            if (j != i)
            {
                int boxX = oldBoxList[j].getX();
                int boxY = oldBoxList[j].getY();
                int boxSize = oldBoxList[j].getSize();
                if (PhysicsEngine::intersectingRectangles(x, newY, size, size, boxX, boxY, boxSize, boxSize, true))
                {
                    colliders.push_back(j);
                    yComponent += (-(size/2 + boxSize/2 + boxY - y))/100 - (yspeed - oldBoxList[j].getYspeed())/2;
                }
            }
        }
        newY += yComponent;
        yspeed += yComponent;

		//check wall collision in Y direction
		if (yspeed > 0) // down
		{
			if (wallmap[x/wallSize][(newY+size)/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][(newY+size)/wallSize]))
			{
				newY = ((newY+size)/wallSize)*wallSize - size;
				supported = true;
			}
		}
		else if (yspeed < 0) // up
		{
			if  (wallmap[x/wallSize][newY/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][newY/wallSize]))
			{
				newY = (newY/wallSize + 1)*wallSize;
			}
		}

        // ** X component **
        int newX = x + xspeed;

        int xComponent = 0;
        for (int j = oldBoxList.size()-1; j >= 0; --j)
        {
            if (j != i)
            {
                if (colliders.size() > 0 && colliders.back() == j)
                {
                    colliders.pop_back();
                }
                else
                {
                    int boxX = oldBoxList[j].getX();
                    int boxY = oldBoxList[j].getY();
                    int boxSize = oldBoxList[j].getSize();
                    if (PhysicsEngine::intersectingRectangles(newX, newY+400, size, size-800, boxX, boxY, boxSize, boxSize, true))
                    {
                        xComponent += (-(size/2 + boxSize/2 + boxX - x))/100 - (xspeed - oldBoxList[j].getXspeed())/2;
                    }
                }
            }
        }

        newX += xComponent;
        xspeed += xComponent;

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
                    oldBoxList[i].getTimeDirection()
                )
            ,supported
            )
        );

		// don't bother checking box collision in crappy step
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
