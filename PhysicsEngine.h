#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H
#include "ObjectList.h"
#include "InputList.h"
#include "TimeObjectListList.h"
#define BOOST_SP_DISABLE_THREADS
#include <boost/smart_ptr.hpp>
#include <vector>
namespace hg {
class PhysicsEngine
{

public:

	PhysicsEngine(int newTimeLineLength, 
                  std::vector<std::vector<bool> > newWallmap,
                  int newWallSize, 
                  int newGravity);

    // executes frame and returns departures
	boost::shared_ptr<hg::TimeObjectListList> executeFrame(const hg::ObjectList& arrivals,
                                                       int time, 
                                                       int playerGuyIndex, 
                                                       const std::vector<boost::shared_ptr<hg::InputList> >& playerInput); 

	inline int getNextPlayerFrame() {return nextPlayerFrame;}
	inline int getPlayerDirection() {return playerDirection;}
private:

	void crappyBoxCollisionAlogorithm(std::vector<boost::shared_ptr<hg::Box> > oldBoxList);
	void guyStep(const std::vector<boost::shared_ptr<hg::Guy> >& oldGuyListt, 
                 int playerGuyIndex, int time, 
                 const std::vector<boost::shared_ptr<hg::InputList> >& playerInput, 
                 boost::shared_ptr<hg::TimeObjectListList> departures);

	bool wallAt(int x, int y);

	int timeLineLength;

    std::vector<std::vector<bool> > wallmap;

	// these are updated from guy with relative index == playerGuyIndex when the frame is executed
	int nextPlayerFrame; // frame that the player departed for
	int playerDirection; // time direction of player in frame
	int gravity;
	int wallSize;

    std::vector<boost::shared_ptr<hg::Box> > nextBox;
    std::vector<int> nextBoxTime;
    std::vector<bool> supportedBox;
};
}
#endif //HG_PHYSICS_ENGINE_H
