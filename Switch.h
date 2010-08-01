#ifndef HG_SWITCH_H
#define HG_SWITCH_H
namespace hg {
    
class Switch
{

public:
    Switch(int x, int y, int type ,int platformAttachment, int id);
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

	int type;
	int platformAttachment;
	int id;
     
};
}
#endif //HG_SWITCH_H