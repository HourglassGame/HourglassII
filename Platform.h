#ifndef HG_PLATFORM_H
#define HG_PLATFORM_H
namespace hg {
class Platform
{
public:
    Platform(int x, int y, int xspeed, int yspeed, int timeDirection, int id);
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

	int id;
    int timeDirection;
};
}
#endif //HG_PLATFORM_H