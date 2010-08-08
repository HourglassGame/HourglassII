#ifndef HG_GUY_H
#define HG_GUY_H
#include "TimeDirection.h"
namespace hg {
class Guy
{
public:
    Guy(int x,
        int y, 
        int xspeed,
        int yspeed, 
        int width, 
        int height, 
        hg::TimeDirection timeDirection, 
        bool boxCarrying, 
        int nBoxCarrySize, 
        hg::TimeDirection nBoxCarryDirection, 
        int relativeIndex, 
        int subimage);
	
    inline int getX() const {return x;}
    inline int getY() const {return y;}
    inline int getXspeed() const {return xspeed;}
    inline int getYspeed() const {return yspeed;}
	inline int getWidth() const {return width;}
	inline int getHeight() const {return height;}

	inline bool getBoxCarrying() const {return boxCarrying;}
	inline int getBoxCarrySize() const {return boxCarrySize;}
	inline hg::TimeDirection getBoxCarryDirection() const {return boxCarryDirection;}

	inline hg::TimeDirection getTimeDirection() const {return timeDirection;}
	inline int getRelativeIndex() const {return relativeIndex;}
	inline int getSubimage() const {return subimage;}

	const static int animationLength = 13;

    bool operator!=(const Guy& other) const;
    bool operator==(const Guy& other) const;
    
	bool operator<(const Guy& second) const;
private:
	int x;
	int y;
	int xspeed;
	int yspeed;
	int width;
	int height;

	bool boxCarrying;
	int boxCarrySize;
    hg::TimeDirection boxCarryDirection;

    hg::TimeDirection timeDirection;
	int relativeIndex;

	int subimage;
};
}
#endif //HG_GUY_H
