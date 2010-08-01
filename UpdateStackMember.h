#ifndef HG_UPDATE_STACK_MEMBER_H
#define HG_UPDATE_STACK_MEMBER_H
namespace hg {
class UpdateStackMember
{
public:
    enum Type {
        CLEAR_PARADOX = 0,
        EXECUTE_FRAME = 1
    };
	UpdateStackMember(Type type, int frame);

	inline int getFrame() const {return frame;}
	inline Type getType() const {return type;}

private:

	int frame;
	Type type; // 0 = paradox clear, 1 = execute frame

};
}
#endif //HG_UPDATE_STACK_MEMBER_H
