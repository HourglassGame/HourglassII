#include "PhysicsEngine.h"


PhysicsEngine::PhysicsEngine(int newTimeLineLength, vector<vector<bool> > newWallmap, int newWallSize, int newGravity)
{
	
	timeLineLength = newTimeLineLength;

	wallmap = newWallmap;
	wallSize = newWallSize;
	gravity = newGravity;

}

boost::shared_ptr<TimeObjectListList> PhysicsEngine::executeFrame(const ObjectList& arrivals, int time, int playerGuyIndex, vector<boost::shared_ptr<InputList> > playerInput)
{
	nextBox.clear();
	nextBoxTime.clear();
	supportedBox.clear();

	boost::shared_ptr<TimeObjectListList> newDepartures = boost::shared_ptr<TimeObjectListList>(new TimeObjectListList());

	// Switch Collisions at this point?

	// Trigger system execution

	// platforms set their new location and veloctiy from trigger system data (and ofc their physical data)

	// switch position update
	// pickup position update

	crappyBoxCollisionAlogorithm(arrivals.getBoxList());
	// boxes do their crazy wizz-bang collision algorithm

	// item simple collision algorithm

	// guys simple collision algorithm
	guyStep(arrivals.getGuyList(), playerGuyIndex, time, playerInput, newDepartures);

	// guys pickup pickups

	// guys pickup/put down boxes and objects
	
	// guys do timetravel-type stuff

	for (unsigned int i = 0; i < nextBox.size(); ++i)
	{
		int nextTime = time+nextBox[i]->getTimeDirection();

		if (nextTime >= 0 && nextTime < timeLineLength)
		{
			newDepartures->getObjectListForManipulation(nextTime)->addBox(nextBox[i]);
		}
	}

	// add data to departures

	return newDepartures;
}

int PhysicsEngine::getNextPlayerFrame()
{
	return nextPlayerFrame;
}

int PhysicsEngine::getPlayerDirection()
{
	return playerDirection;
}

bool PhysicsEngine::wallAt(int x, int y)
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

void PhysicsEngine::guyStep(vector<boost::shared_ptr<Guy> > oldGuyList, int playerGuyIndex, int time, vector<boost::shared_ptr<InputList> > playerInput, boost::shared_ptr<TimeObjectListList> newDepartures)
{
	vector<int> x;
	vector<int> y;
	vector<int> xspeed;
	vector<int> yspeed;

	// position, velocity, collisions
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		x.push_back(oldGuyList[i]->getX());
		y.push_back(oldGuyList[i]->getY());
		xspeed.push_back(0);
		yspeed.push_back(oldGuyList[i]->getYspeed() + gravity);

		int relativeIndex = oldGuyList[i]->getRelativeIndex();
		boost::shared_ptr<InputList> input = playerInput[relativeIndex];

		int width = oldGuyList[i]->getWidth();
		int height = oldGuyList[i]->getHeight();
		bool supported = false;

		if (input->getLeft())
		{
			xspeed[i] = -150;
		}
		else if (input->getRight())
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

		// jump
		if (supported && input->getUp())
		{	
			yspeed[i] = -800;
		}
	}

	vector<bool> carry;
	vector<int> carrySize;
	vector<int> carryDirection;
	
	// box carrying
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		carry.push_back(oldGuyList[i]->getBoxCarrying());
		carrySize.push_back(0);
		carryDirection.push_back(0);

		int relativeIndex = oldGuyList[i]->getRelativeIndex();
		boost::shared_ptr<InputList> input = playerInput[relativeIndex];

		if (carry[i])
		{
			if (input->getDown())
			{ 
				int width = oldGuyList[i]->getWidth();
				int height = oldGuyList[i]->getHeight();

				// fixme: needs to check if the box can be dropped
				nextBox.push_back(boost::shared_ptr<Box>(new Box(x[i] + width/2 - carrySize[i]/2, y[i] - height - carrySize[i], 0, 0, oldGuyList[i]->getBoxCarrySize(), oldGuyList[i]->getBoxCarryDirection())));
				supportedBox.push_back(false);
				carry[i] = false;
				carrySize[i] = 0;
				carryDirection[i] = 0;
			}
			else
			{
				carrySize[i] = oldGuyList[i]->getBoxCarrySize();
				carryDirection[i] = oldGuyList[i]->getBoxCarryDirection();
			}
		}
		else
		{
			if (input->getDown())
			{
				int width = oldGuyList[i]->getWidth();
				int height = oldGuyList[i]->getHeight();

				for (unsigned int j = 0; j < nextBox.size(); ++j)
				{
					int boxX = nextBox[j]->getX();
					int boxY = nextBox[j]->getY();
					int boxSize = nextBox[j]->getSize();
					if ((x[i] < boxX+boxSize) && (x[i]+width > boxX) && (y[i] < boxY+boxSize)&& (y[i]+height > boxY)) 
					{
						carry[i] = true;
						carrySize[i] = boxSize;
						carryDirection[i] = nextBox[j]->getTimeDirection();
						nextBox.erase(nextBox.begin() + j);
					}
				   
				}
			}
			else
			{
				carrySize[i] = 0;
				carryDirection[i] = 0;
			}
		}
	}


	// colliding with pickups?


	// shooting, jumping


	// write to departure arrays, other little things
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		// animation
		int nextSubimage = oldGuyList[i]->getSubimage() + 1;
		if (nextSubimage > Guy::animationLength)
		{
			nextSubimage = 0;
		}

		int relativeIndex = oldGuyList[i]->getRelativeIndex();

		// add departure for guy at the appropriate time
		int nextTime = time+oldGuyList[i]->getTimeDirection();
		if (nextTime >= 0 && nextTime < timeLineLength)
		{
			newDepartures->getObjectListForManipulation(nextTime)->addGuy(x[i], y[i], xspeed[i], yspeed[i], oldGuyList[i]->getWidth(), oldGuyList[i]->getHeight(), 
				oldGuyList[i]->getTimeDirection(), carry[i], carrySize[i], carryDirection[i], relativeIndex+1, nextSubimage);
		}
		
		if (playerGuyIndex == relativeIndex) // this means the player is controlling this guy.
		{
			playerDirection = oldGuyList[i]->getTimeDirection(); // controls propgation order for time directional symetry 
			nextPlayerFrame = nextTime; // frame that will be changed by adding input next getPlayerFrame
		}
	}

}

void PhysicsEngine::crappyBoxCollisionAlogorithm(vector<boost::shared_ptr<Box> > oldBoxList)
{

	for (unsigned int i = 0; i < oldBoxList.size(); ++i)
	{
		int x = oldBoxList[i]->getX();
		int y = oldBoxList[i]->getY();
		int xspeed = oldBoxList[i]->getXspeed();
		int yspeed = oldBoxList[i]->getYspeed() + gravity;
		int size = oldBoxList[i]->getSize();

		bool supported = false;

		x = x + xspeed;
		y = y + yspeed;

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

		nextBox.push_back(boost::shared_ptr<Box>(new Box(x, y, xspeed, yspeed, size, oldBoxList[i]->getTimeDirection())));
		supportedBox.push_back(supported);

		// don't bother checking box collision in crappy step

	}

}