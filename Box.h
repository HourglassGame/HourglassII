#include <boost/smart_ptr.hpp>

class Box
{

public:
    Box(int x, int y, int xspeed, int yspeed, int size, int timeDirection);

    int getX();
    int getY();
    int getXspeed();
    int getYspeed();
	int getSize();
	int getTimeDirection();

	bool equals(boost::shared_ptr<Box> other);
    static bool lessThan(boost::shared_ptr<Box> first, boost::shared_ptr<Box> second);

private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;
	int size;

	int timeDirection;
     
};