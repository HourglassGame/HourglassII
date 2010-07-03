#include <boost/smart_ptr.hpp>

class Guy
{

public:
    Guy(int x, int y, int xspeed, int yspeed, int timeDirection, bool boxCarrying, int relativeIndex);
	
    int getX();
    int getY();
    int getXspeed();
    int getYspeed();

	bool getBoxCarrying();
	int getTimeDirection();
	int getRelativeIndex();

	bool equals(boost::shared_ptr<Guy> other);
	static bool lessThan(boost::shared_ptr<Guy> first, boost::shared_ptr<Guy> second);
    
private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;

	bool boxCarrying;
	int timeDirection;
	int relativeIndex;
     
};
