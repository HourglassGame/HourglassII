#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
namespace hg {
class Box
{

public:
    Box(int x, int y, int xspeed, int yspeed, int size, hg::TimeDirection timeDirection);

    inline int getX() const {return x;}
    inline int getY() const {return y;}
    inline int getXspeed() const {return xspeed;}
    inline int getYspeed() const {return yspeed;}
	inline int getSize() const {return size;}
	inline hg::TimeDirection getTimeDirection() const {return timeDirection;}

	static const int normalSize = 1000;

	bool equals(const Box& other) const;
    bool operator<(const Box& second) const;

private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;
	int size;

    hg::TimeDirection timeDirection;
     
};
}
#endif //HG_BOX_H
