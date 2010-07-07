#include "PhysicsEngine.h"


PhysicsEngine::PhysicsEngine(int newTimeLineLength, vector<vector<bool> > newWallmap, int newWallSize, int newGravity)
{
	
	timeLineLength = newTimeLineLength;

	wallmap = newWallmap;
	wallSize = newWallSize;
	gravity = newGravity;

}

vector<boost::shared_ptr<ObjectList> > PhysicsEngine::executeFrame(boost::shared_ptr<ObjectList> arrivals, int time, int playerGuyIndex, vector<boost::shared_ptr<InputList> > playerInput)
{
	nextBox.clear();
	nextBoxTime.clear();
	supportedBox.clear();

	nextGuy.clear();
	nextGuyTime.clear();

	vector<boost::shared_ptr<ObjectList> > departures;
	for (int i = 0; i < timeLineLength; ++i)
	{
		departures.push_back(boost::shared_ptr<ObjectList>(new ObjectList()));
	}

	// Switch Collisions at this point?

	// Trigger system execution

	// platforms set their new location and veloctiy from trigger system data (and ofc their physical data)

	// switch position update
	// pickup position update

	crappyBoxCollisionAlogorithm(arrivals->getBoxList());
	// boxes do their crazy wizz-bang collision algorithm

	// item simple collision algorithm

	// guys simple collision algorithm
	guyStep(arrivals->getGuyList(), playerGuyIndex, time, playerInput, departures);

	// guys pickup pickups

	// guys pickup/put down boxes and objects
	
	// guys do timetravel-type stuff

	for (unsigned int i = 0; i < nextBox.size(); ++i)
	{
		int nextTime = time+nextBox[i]->getTimeDirection();

		if (nextTime >= 0 && nextTime < timeLineLength)
		{
			departures[nextTime]->addBox(nextBox[i]);
		}
	}

	// add data to departures

	return departures;
}

int PhysicsEngine::getNextPlayerFrame()
{
	return nextPlayerFrame;
}

int PhysicsEngine::getPlayerDirection()
{
	return playerDirection;
}

void PhysicsEngine::guyStep(vector<boost::shared_ptr<Guy> > oldGuyList, int playerGuyIndex, int time, vector<boost::shared_ptr<InputList> > playerInput, vector<boost::shared_ptr<ObjectList> > departures)
{
	
	for (unsigned int i = 0; i < oldGuyList.size(); ++i)
	{
		int x = oldGuyList[i]->getX();
		int y = oldGuyList[i]->getY();
		int xspeed;
		int yspeed = oldGuyList[i]->getYspeed() + gravity;
		int width = oldGuyList[i]->getWidth();
		int height = oldGuyList[i]->getHeight();
		
		bool supported = false;

		int relativeIndex = oldGuyList[i]->getRelativeIndex();

		boost::shared_ptr<InputList> input = playerInput[relativeIndex];
		if (input->getLeft())
		{
			xspeed = -150;
		}
		else if (input->getRight())
		{
			xspeed = 150;
		}
		else
		{
			xspeed = 0;
		}

		//check wall collision in Y direction
		y = y + yspeed;

		if (yspeed > 0) // down
		{
			if (wallmap[x/wallSize][(y+height)/wallSize] || (x - (x/wallSize)*wallSize > wallSize-width && wallmap[(x+width)/wallSize][(y+height)/wallSize]))
			{
				yspeed = 0;
				y = ((y+height)/wallSize)*wallSize - height;
				supported = true;
			}
		}
		else if (yspeed < 0) // up
		{
			if  (wallmap[x/wallSize][y/wallSize] || (x - (x/wallSize)*wallSize > wallSize-width && wallmap[(x+width)/wallSize][y/wallSize]))
			{
				yspeed = 0;
				y = (y/wallSize + 1)*wallSize;
			}
		}

		//check wall collision in X direction
		x = x + xspeed;

		if (xspeed > 0) // right
		{
			if (wallmap[(x+width)/wallSize][y/wallSize] || (y - (y/wallSize)*wallSize > wallSize-height && wallmap[(x+width)/wallSize][(y+height)/wallSize]))
			{
				xspeed = 0;
				x = (x+width)/wallSize*wallSize - width;
			}
		}
		else if (xspeed < 0) // left
		{
			if (wallmap[x/wallSize][y/wallSize] || (y - (y/wallSize)*wallSize > wallSize-height && wallmap[x/wallSize][(y+height)/wallSize]))
			{
				xspeed = 0;
				x = (x/wallSize + 1)*wallSize;
			}
		}

		// jump
		if (supported && input->getUp())
		{	
			yspeed = -800;
		}

		// box carrying
		bool carry = oldGuyList[i]->getBoxCarrying();
		int carryDirection = oldGuyList[i]->getBoxCarryDirection();

		if (input->getDown())
		{
			if (carry)
			{


			}
			else
			{

			}
		}

		// update animation frame
		int nextSubimage = oldGuyList[i]->getSubimage() + 1;
		if (nextSubimage > Guy::animationLength)
		{
			nextSubimage = 0;
		}

		// add guy to physicsEngine array
		nextGuy.push_back(boost::shared_ptr<Guy>(new Guy(x, y, xspeed, yspeed, width, height, oldGuyList[i]->getTimeDirection(), carry, carryDirection, relativeIndex+1, nextSubimage)));

		// add departure for guy at the appropriate time
		int nextTime = time+nextGuy[i]->getTimeDirection();
		if (nextTime >= 0 && nextTime < timeLineLength)
		{
			departures[nextTime]->addGuy(nextGuy[i]);
		}
		
		if (playerGuyIndex == relativeIndex)
		{
			// this means the player is controlling this guy.
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