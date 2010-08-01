#ifndef HG_INPUT_LIST_H
#define HG_INPUT_LIST_H
namespace hg {
class InputList
{
	
public:
	InputList(bool left,
              bool right, 
              bool up,
              bool down,
              bool space,
              bool mouseLeft,
              bool mouseRight,
              int mouseX,
              int mouseY);

	inline bool getLeft() const {return left;}
	inline bool getRight() const {return right;}
	inline bool getUp() const {return up;}
	inline bool getDown() const {return down;}
	inline bool getSpace() const {return space;}
	inline bool getMouseLeft() const {return mouseLeft;}
	inline bool getMouseRight() const {return mouseRight;}
	inline int getMouseX() const {return mouseX;}
	inline int getMouseY() const {return mouseY;}

private:

	bool left;
	bool right;
	bool up;
	bool down;
	bool space;

	bool mouseLeft;
	bool mouseRight;
	int mouseX;
	int mouseY;

};
}
#endif //HG_INPUT_LIST_H
