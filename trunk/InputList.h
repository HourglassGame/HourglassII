#ifndef HG_INPUT_LIST_H
#define HG_INPUT_LIST_H
#include "FrameID.h"
#include "Ability.h"

#include <cassert>

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace hg {
class InputList
{
public:
    InputList(
        bool left,
        bool right,
        bool up,
        bool down,
        bool use,
        Ability ability,
        FrameID abilityParams,
        int frameIdParamCount);

    bool getLeft()       const { return left; }
    bool getRight()      const { return right; }
    bool getUp()         const { return up; }
    bool getDown()       const { return down; }
    bool getUse()        const { return use; }
    Ability getAbility() const { return ability; }

    FrameID getFrameIdParam(int param) const
    {
        if (param >= 0 && param < frameIdParamCount) {
            return frameIdParams;
        }
        // throw exception here.
        assert(false);
        return FrameID();
    }

private:
    InputList() :
        left(false),
        right(false),
        up(false),
        down(false),
        use(false),
        ability(NO_ABILITY),
        frameIdParams(),
        frameIdParamCount(0)
    {
    }
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        (void)version;
        ar & left;
        ar & right;
        ar & up;
        ar & down;
        ar & use;
        ar & ability;
        ar & frameIdParams;
        ar & frameIdParamCount;
    }
    
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
