
class Guy
{

public:
    Guy(int x, int y, int xspeed, int yspeed, int timeDirection, bool boxCarrying, int relativeIndex);
	/*
    int getX();
    int getY();
    int getXspeed();
    int getYspeed();
	*/
	// exectute physics

	// execute time things (belts, jumps)
    
private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;

	bool boxCarrying;
	int timeDirection;
	int relativeIndex;
     
};
