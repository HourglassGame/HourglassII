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
InputList(){}
	bool left;
	bool right;
	bool up;
	bool down;
	bool use;

	Ability ability;
	NewFrameID frameIdParams;
	int frameIdParamCount;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        printf("y0");
        ar & left;
        ar & right;
        ar & up;
        ar & down;
        ar & use;

        ar & ability;
        ar & frameIdParams;
        ar & frameIdParamCount;
    }

};
}
#endif //HG_INPUT_LIST_H
