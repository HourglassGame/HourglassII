#ifndef HG_ITEM_H
#define HG_ITEM_H
namespace hg {
class Item
{
public:
    Item(int x, int y, int xspeed, int yspeed, int timeDirection, int type);
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
	int type;
     
};
}
#endif //HG_ITEM_H
