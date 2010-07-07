#include <boost/smart_ptr.hpp>

class Guy
{

public:
    Guy(int x, int y, int xspeed, int yspeed, int width, int height, int timeDirection, bool boxCarrying, int relativeIndex, int subimage);
	
    int getX();
    int getY();
    int getXspeed();
    int getYspeed();
	int getWidth();
	int getHeight();

	bool getBoxCarrying();
	int getTimeDirection();
	int getRelativeIndex();
	int getSubimage();

	static int animationLength;

	bool equals(boost::shared_ptr<Guy> other);
	static bool lessThan(boost::shared_ptr<Guy> first, boost::shared_ptr<Guy> second);
    
private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;
	int width;
	int height;

	bool boxCarrying;
	int timeDirection;
	int relativeIndex;

	int subimage;
     
};
