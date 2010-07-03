
class UpdateStackMember
{
public:
	UpdateStackMember(int type, int frame);

	int getFrame();
	int getType();

private:

	int frame;
	int type; // 0 = paradox clear, 1 = execute frame

};