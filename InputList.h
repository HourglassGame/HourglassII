
class InputList
{
	
public:
	InputList(bool left, bool right, bool up, bool down, bool space, bool mouseLeft, bool mouseRight, int mouseX, int mouseY);

	bool getLeft();
	bool getRight();
	bool getUp();
	bool getDown();
	bool getSpace();
	bool getMouseLeft();
	bool getMouseRight();
	int getMouseX();
	int getMouseY();

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