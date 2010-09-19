#include "NewFrameID.h"
#include "Ability.h"
#include <cassert>

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
              bool use,
              Ability ability,
              NewFrameID abilityParams,
              int frameIdParamCount);

    bool getLeft() const {return left;}
	bool getRight() const {return right;}
	bool getUp() const {return up;}
	bool getDown() const {return down;}
	bool getUse() const {return use;}
	Ability getAbility() const {return ability;}

	NewFrameID getFrameIdParam(int param) const
    {
        if (param >= 0 && param < frameIdParamCount)
        {
            return frameIdParams;
        }
        // throw exception here.
        assert(false);
        return NewFrameID();
    }

private:

	bool left;
	bool right;
	bool up;
	bool down;
	bool use;

	Ability ability;
	NewFrameID frameIdParams;
	int frameIdParamCount;

};
}
#endif //HG_INPUT_LIST_H
