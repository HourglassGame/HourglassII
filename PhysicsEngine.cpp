#include "PhysicsEngine.h"

#include "TimeObjectListList.h"
#include "ObjectList.h"
#include "TimeDirection.h"

#include <iostream>
#include <cassert>
#include <map>

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
            newDepartures, nextBox, currentPlayerFrame, nextPlayerFrame);

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

void PhysicsEngine::guyStep(const vector<Guy>& oldGuyList,
                            const FrameID time,
                            const vector<InputList>& playerInput,
                            map<FrameID, MutableObjectList>& newDepartures,
                            std::vector<BoxInfo>& nextBox,
                            FrameID& currentPlayerFrame,
                            FrameID& nextPlayerFrame) const
{
	vector<int> x;
	vector<int> y;
	vector<int> xspeed;
	vector<int> yspeed;

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

            unsigned int relativeIndex = oldGuyList[i].getRelativeIndex();
            const InputList& input = playerInput[relativeIndex];

            int width = oldGuyList[i].getWidth();
            int height = oldGuyList[i].getHeight();
            bool supported = false;

            if (input.getLeft())
            {
                xspeed[i] = -250;
            }
            else if (input.getRight())
            {
                xspeed[i] = 250;
            }
            else
            {
                xspeed[i] = 0;
            }

            //check wall collision in Y direction
            y[i] = y[i] + yspeed[i];

            if (yspeed[i] > 0) // down
            {
                if (wallAt(x[i], y[i]+height) || (x[i] - (x[i]/wallSize)*wallSize > wallSize-width && wallAt(x[i]+width, y[i]+height)))
                {
                    yspeed[i] = 0;
                    y[i] = ((y[i]+height)/wallSize)*wallSize - height;
                    supported = true;
                }
            }
            else if (yspeed[i] < 0) // up
            {
                if  (wallAt(x[i], y[i]) || (x[i] - (x[i]/wallSize)*wallSize > wallSize-width && wallAt(x[i]+width, y[i])))
                {
                    yspeed[i] = 0;
                    y[i] = (y[i]/wallSize + 1)*wallSize;
                }
            }

            //check wall collision in X direction
            x[i] = x[i] + xspeed[i];

            if (xspeed[i] > 0) // right
            {
                if (wallAt(x[i]+width, y[i]) || (y[i] - (y[i]/wallSize)*wallSize > wallSize-height && wallAt(x[i]+width, y[i]+height)))
                {
                    xspeed[i] = 0;
                    x[i] = (x[i]+width)/wallSize*wallSize - width;
                }
            }
            else if (xspeed[i] < 0) // left
            {
                if (wallAt(x[i], y[i]) || (y[i] - (y[i]/wallSize)*wallSize > wallSize-height && wallAt(x[i], y[i]+height)))
                {
                    xspeed[i] = 0;
                    x[i] = (x[i]/wallSize + 1)*wallSize;
                }
            }

            // box collision
            if (yspeed[i] >= 0) // down
            {
                for (unsigned int j = 0; j < nextBox.size(); ++j)
                {
                    int boxX = nextBox[j].box.getX();
                    int boxY = nextBox[j].box.getY();
                    int boxSize = nextBox[j].box.getSize();
                    if (x[i] <= boxX+boxSize and x[i]+width >= boxX and y[i]+height >= boxY and y[i]-yspeed[i]+height <= boxY)
                    {
                        supported = true;
                        yspeed[i] = 0;
                        y[i] = boxY-height;
                    }
                }
            }

            // jump
            if (supported && input.getUp())
            {
                yspeed[i] = -800;
            }
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
                if (input.getDown())
                {
                    int width = oldGuyList[i].getWidth();

                    // fixme: needs to check if the box can be dropped
                    nextBox.push_back
                    (
                     PhysicsEngine::BoxInfo
                     (
                      Box(
                          x[i] + width/2 - oldGuyList[i].getBoxCarrySize()/2,
                          y[i] - oldGuyList[i].getBoxCarrySize(),
                          0,
                          0,
                          oldGuyList[i].getBoxCarrySize(),
                          oldGuyList[i].getBoxCarryDirection()
                          ),
                      false
                      )
                     );
                    carry[i] = false;
                    carrySize[i] = 0;
                    carryDirection[i] = hg::PAUSE;
                }
                else
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
                currentPlayerFrame = time;
                nextPlayerFrame = nextTime;
            }

            if (nextTime >= 0 && nextTime < timeLineLength)
            {
                newDepartures[nextTime].addGuy
                (
                    Guy(x[i], y[i], xspeed[i], yspeed[i],
                        oldGuyList[i].getWidth(), oldGuyList[i].getHeight(),
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
	for (vector<Box>::const_iterator it(oldBoxList.begin()), end(oldBoxList.end()); it != end; ++it)
	{
		int x = it->getX();
		int y = it->getY();
		int xspeed = it->getXspeed();
		int yspeed = it->getYspeed() + gravity;
		int size = it->getSize();

		bool supported = false;

		x += xspeed;
		y += yspeed;

		//check wall collision in Y direction
		if (yspeed > 0) // down
		{
			if (wallmap[x/wallSize][(y+size)/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][(y+size)/wallSize]))
			{
				xspeed = 0;
				yspeed = 0;
				y = ((y+size)/wallSize)*wallSize - size;
				supported = true;
			}
		}
		else if (yspeed < 0) // up
		{
			if  (wallmap[x/wallSize][y/wallSize] || (x - (x/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][y/wallSize]))
			{
				yspeed = 0;
				y = (y/wallSize + 1)*wallSize;
			}
		}

		//check wall collision in X direction
		if (xspeed > 0) // right
		{
			if (wallmap[(x+size)/wallSize][y/wallSize] || (y - (y/wallSize)*wallSize > wallSize-size && wallmap[(x+size)/wallSize][(y+size)/wallSize]))
			{
				xspeed = 0;
				x = (x+size)/wallSize*wallSize - size;
			}
		}
		else if (xspeed < 0) // left
		{
			if (wallmap[x/wallSize][y/wallSize] || (y - (y/wallSize)*wallSize > wallSize-size && wallmap[x/wallSize][(y+size)/wallSize]))
			{
				xspeed = 0;
				x = (x/wallSize + 1)*wallSize;
			}
		}

		// box collision
        if (yspeed >= 0) // down
        {
            for (unsigned int j = 0; j < nextBox.size(); ++j)
            {
                int boxX = nextBox[j].box.getX();
                int boxY = nextBox[j].box.getY();
                int boxSize = nextBox[j].box.getSize();
                if (x <= boxX+boxSize and x+size >= boxX and y+size >= boxY and y-yspeed+size <= boxY)
                {
                    supported = true;
                    yspeed = 0;
                    y = boxY-size;
                }
            }
        }

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
                    it->getTimeDirection()
                )
            ,supported
            )
        );

		// don't bother checking box collision in crappy step
	}
}
