
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
	
	// exectute physics

	// execute time things (belts, jumps)

	bool operator==(Guy* other);
	bool operator!=(Guy* other);
    
private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;

	bool boxCarrying;
	int timeDirection;
	int relativeIndex;
     
};
