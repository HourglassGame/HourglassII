#include "PhysicsEngine.h"


PhysicsEngine::PhysicsEngine(int newTimeLineLength, vector<boost::shared_ptr<InputList>> playerInputRef, vector<vector<bool>> newWallmap)
{
	
	timeLineLength = newTimeLineLength;

	playerInput = playerInputRef;

	wallmap = newWallmap;


}

vector<boost::shared_ptr<ObjectList>> PhysicsEngine::executeFrame(boost::shared_ptr<ObjectList> arrivals, int time, int playerGuyIndex)
{

	// Switch Collisions at this point?

	// Trigger system execution

	// platforms set their new location and veloctiy from trigger system data (and ofc their physical data)

	// switch position update
	// pickup position update

	// boxes do their crazy wizz-bang collision algorithm

	// item simple collision algorithm

	// guys simple collision algorithm

	// guys pickup pickups

	// guys pickup/put down boxes and objects
	
	// guys do timetravel-type stuff

	vector<boost::shared_ptr<ObjectList>> departures(timeLineLength, boost::shared_ptr<ObjectList>(new ObjectList()));

	departures[time]->addBox(1,2,3,4,1);

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