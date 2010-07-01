
class Box
{

public:
    Box(int x, int y, int xspeed, int yspeed, int timeDirection);
	/*
    int getX();
    int getY();
    int getXspeed();
    int getYspeed();
	*/
	// exectute physics
    
private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;

	int timeDirection;
     
};