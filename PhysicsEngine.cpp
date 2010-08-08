#include "PhysicsEngine.h"
#include "TimeDirection.h"

using namespace std;
using namespace hg;

PhysicsEngine::PhysicsEngine(int newTimeLineLength, vector<vector<bool> > newWallmap, int newWallSize, int newGravity)
{
	
	timeLineLength = newTimeLineLength;

	wallmap = newWallmap;
	wallSize = newWallSize;
	gravity = newGravity;

}


//NOTE - no time travel yet so currentPlayerFrame does not ever change.
TimeObjectListList PhysicsEngine::executeFrame(const hg::ObjectList& arrivals,
                                               int time,
                                               const std::vector<hg::InputList>& playerInput,
                                               int& currentPlayerFrame,
                                               int& nextPlayerFrame) const
{
    std::vector<PhysicsEngine::BoxInfo> nextBox;

	TimeObjectListList newDepartures;

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

	for (unsigned int i = 0; i < nextBox.size(); ++i)
	{
		int nextTime = time+nextBox[i].box.getTimeDirection();

		if (nextTime >= 0 && nextTime < timeLineLength)
		{
			newDepartures.getObjectListForManipulation(nextTime).addBox(nextBox[i].box);
		}
	}

	// add data to departures

	return newDepartures;
}

bool PhysicsEngine::wallAt(int x, int y) const
{
	if (x < 0 || y < 0)
	{
		return true;
	}

	unsigned int aX = x/wallSize;
	unsigned int aY = y/wallSize;

	if ( aX < wallmap.size() && aY < wallmap[aX].size())
	{
		return wallmap[aX][aY];
	}
	else
	{
		return true;
	}
}

void PhysicsEngine::guyStep(const vector<Guy>& oldGuyList, const int time, 
                            const vector<InputList>& playerInput, TimeObjectListList& newDepartures,
                            std::vector<PhysicsEngine::BoxInfo>& nextBox,
                            int& currentPlayerFrame,
                            int& nextPlayerFrame) const
{
	vector<int> x;
	vector<int> y;
	vector<int> xspeed;
	vector<int> yspeed;

	// position, velocity, collisions
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		x.push_back(oldGuyList[i].getX());
		y.push_back(oldGuyList[i].getY());
		xspeed.push_back(0);
		yspeed.push_back(oldGuyList[i].getYspeed() + gravity);

		int relativeIndex = oldGuyList[i].getRelativeIndex();
		const InputList& input = playerInput[relativeIndex];

		int width = oldGuyList[i].getWidth();
		int height = oldGuyList[i].getHeight();
		bool supported = false;

		if (input.getLeft())
		{
			xspeed[i] = -150;
		}
		else if (input.getRight())
		{
			xspeed[i] = 150;
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
        
		// jump
		if (supported && input.getUp())
		{	
			yspeed[i] = -800;
		}
	}

	vector<bool> carry;
	vector<int> carrySize;
	vector<hg::TimeDirection> carryDirection;
	
	// box carrying
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		carry.push_back(oldGuyList[i].getBoxCarrying());
		carrySize.push_back(0);
		carryDirection.push_back(hg::PAUSE);

		int relativeIndex = oldGuyList[i].getRelativeIndex();
		const InputList& input = playerInput[relativeIndex];

		if (carry[i])
		{
			if (input.getDown())
			{ 
				int width = oldGuyList[i].getWidth();
				int height = oldGuyList[i].getHeight();

				// fixme: needs to check if the box can be dropped
				nextBox.push_back
                (
                    PhysicsEngine::BoxInfo
                    (
                        Box(
                            x[i] + width/2 - carrySize[i]/2,
                            y[i] - height - carrySize[i], 
                            0,
                            0,
                            oldGuyList[i].getBoxCarrySize(),
                            oldGuyList[i].getBoxCarryDirection()
                        ),
                        false
                    )
                );
				//supportedBox.push_back(false);
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


	// colliding with pickups?


	// shooting, jumping


	// write to departure arrays, other little things
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		// animation
		int nextSubimage = oldGuyList[i].getSubimage() + 1;
		if (nextSubimage > Guy::animationLength)
		{
			nextSubimage = 0;
		}

		int relativeIndex = oldGuyList[i].getRelativeIndex();

		// add departure for guy at the appropriate time
		int nextTime = time+oldGuyList[i].getTimeDirection();
        
        if ((playerInput.size()) != relativeIndex) // this means that this guy has already recieved input and so can have physics
		{
            if (playerInput.size() - 1 == relativeIndex) {
                currentPlayerFrame = time;
                nextPlayerFrame = nextTime;
            }
            if (nextTime >= 0 && nextTime < timeLineLength)
            {
                newDepartures.getObjectListForManipulation(nextTime).addGuy(Guy(x[i], y[i], xspeed[i], yspeed[i],
                                                                        oldGuyList[i].getWidth(), oldGuyList[i].getHeight(), 
                                                                        oldGuyList[i].getTimeDirection(),
                                                                        carry[i], carrySize[i], carryDirection[i],
                                                                        relativeIndex+1, nextSubimage));
            }
		}
	}

}

void PhysicsEngine::crappyBoxCollisionAlogorithm(const vector<hg::Box>& oldBoxList,
                                                 std::vector<PhysicsEngine::BoxInfo>& nextBox) const
{
	for (unsigned int i = 0; i < oldBoxList.size(); ++i)
	{
		int x = oldBoxList[i].getX();
		int y = oldBoxList[i].getY();
		int xspeed = oldBoxList[i].getXspeed();
		int yspeed = oldBoxList[i].getYspeed() + gravity;
		int size = oldBoxList[i].getSize();

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
		//supportedBox.push_back(supported);

		// don't bother checking box collision in crappy step

	}

}