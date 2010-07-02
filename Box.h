
class Box
{

public:
    Box(int x, int y, int xspeed, int yspeed, int timeDirection);

    int getX();
    int getY();
    int getXspeed();
    int getYspeed();
	int getTimeDirection();
	
	// exectute physics

	bool operator==(Box* other);
	bool operator!=(Box* other);
    
private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;

	int timeDirection;
     
};