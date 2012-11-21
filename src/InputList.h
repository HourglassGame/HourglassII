#ifndef HG_INPUT_LIST_H
#define HG_INPUT_LIST_H
#include "FrameID.h"
#include "Ability.h"

#include <boost/operators.hpp>
#include <istream>
#include <ostream>
#include <boost/serialization/version.hpp>

#include <cassert>

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace hg {
class InputList : boost::equality_comparable<InputList>
{
public:
    InputList(
        bool left,
        bool right,
        bool up,
        bool down,
        bool use,
        Ability ability,
        FrameID timeParam,
        int xParam,
        int yParam);

    bool operator==(InputList const& o) const
    {
        return left == o.left
            && right == o.right
            && up == o.up
            && down == o.down
            && use == o.use
            && ability == o.ability
            && timeParam == o.timeParam
            && xParam == o.xParam
            && yParam == o.yParam;
    }

    bool getLeft()       const { return left; }
    bool getRight()      const { return right; }
    bool getUp()         const { return up; }
    bool getDown()       const { return down; }
    bool getUse()        const { return use; }
    Ability getAbility() const { return ability; }

    FrameID getTimeParam() const { return timeParam; }
    
    int getXParam() const { return xParam; }
    int getYParam() const { return yParam; }
    
    InputList() :
        left(),
        right(),
        up(),
        down(),
        use(),
        ability(NO_ABILITY),
        timeParam(),
        xParam(),
        yParam()
    {
    }
private:

    
    //Crappy serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & left;
        ar & right;
        ar & up;
        ar & down;
        ar & use;
        ar & ability;
        ar & timeParam;
        if (version == 0) {
            int frameIdParamCount;
            ar & frameIdParamCount;
        }
        if (version >= 1) {
            ar & xParam;
            ar & yParam;
        }
    }
    
    bool left;
    bool right;
    bool up;
    bool down;
    bool use;

    Ability ability;
    FrameID timeParam;
    int xParam;
    int yParam;
    
    //more crappy serialization
    inline friend std::ostream& operator<<(std::ostream& os, InputList const& toPrint)
    {
        os << toPrint.left << " ";
        os << toPrint.right << " ";
        os << toPrint.up << " ";
        os << toPrint.down << " ";
        os << toPrint.use << " ";
        os << toPrint.ability << " ";
        os << toPrint.timeParam << " ";
        os << toPrint.xParam << " ";
        os << toPrint.yParam;
        return os;
    }
    inline friend std::istream& operator>>(std::istream& is, InputList& toRead)
    {
        is >> toRead.left;
        is >> toRead.right;
        is >> toRead.up;
        is >> toRead.down;
        is >> toRead.use;
        int ability;
        is >> ability;
        toRead.ability = static_cast<Ability>(ability);
        is >> toRead.timeParam;
        is >> toRead.xParam;
        is >> toRead.yParam;
        return is;
    }
};
}
BOOST_CLASS_VERSION(hg::InputList, 1)
#endif //HG_INPUT_LIST_H
