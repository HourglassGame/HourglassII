#include "FrameID.h"
#include "Ability.h"
#include <cassert>
#include <iostream>

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
              FrameID abilityParams,
              int frameIdParamCount);

    inline bool getLeft() const {return left;}
	inline bool getRight() const {return right;}
	inline bool getUp() const {return up;}
	inline bool getDown() const {return down;}
	inline bool getUse() const {return use;}
	inline Ability getAbility() const {return ability;}

	inline FrameID getFrameIdParam(int param) const
    {
        if (param >= 0 and param < frameIdParamCount)
        {
            return frameIdParams;
        }
        // throw exception here.
        assert(false);
        return 0;
    }

private:

	bool left;
	bool right;
	bool up;
	bool down;
	bool use;

	Ability ability;
	FrameID frameIdParams;
	int frameIdParamCount;

};
}
#endif //HG_INPUT_LIST_H
