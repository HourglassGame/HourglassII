
class Pickup
{

public:
    Pickup(int x, int y, int timeDirection, int type ,int platformAttachment);
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

	int timeDirection;
	int type;
	int platformAttachment;
     
};