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
class InputList;
std::ostream &operator<<(std::ostream &os, InputList const &toPrint);
std::istream &operator>>(std::istream &is, InputList &toRead);
class InputList final : boost::equality_comparable<InputList>
{
public:
    InputList() noexcept;
    InputList(
        bool left,
        bool right,
        bool up,
        bool down,
        bool portalUsed,
        bool abilityUsed,
        Ability abilityCursor,
        FrameID timeCursor,
        int xCursor,
        int yCursor) noexcept;

    bool operator==(InputList const &o) const;

    bool getLeft()       const { return left; }
    bool getRight()      const { return right; }
    bool getUp()         const { return up; }
    bool getDown()       const { return down; }
    bool getPortalUsed() const { return portalUsed; }
    
    bool getAbilityUsed() const { return abilityUsed; }
    Ability getAbilityCursor() const { return abilityCursor; }
    FrameID getTimeCursor() const { return timeCursor; }
    int getXCursor() const { return xCursor; }
    int getYCursor() const { return yCursor; }

private:
    //Crappy serialization
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive & ar, unsigned int const version)
    {
        ar & left;
        ar & right;
        ar & up;
        ar & down;
        ar & portalUsed;
        if (version < 2) {
            //Loading only, as version < current version.
            ar & abilityCursor;
            abilityUsed = abilityCursor != Ability::NO_ABILITY;
            ar & timeCursor;
            if (version == 0) {
                int frameIdParamCount;
                ar & frameIdParamCount;
            }
            if (version >= 1) {
                ar & xCursor;
                ar & yCursor;
            }
        }
        else {
            ar & abilityUsed;
            ar & abilityCursor;
            ar & timeCursor;
            ar & xCursor;
            ar & yCursor;
        }
    }

    bool left;
    bool right;
    bool up;
    bool down;
    bool portalUsed;
    bool abilityUsed;
    Ability abilityCursor;
    FrameID timeCursor;
    int xCursor;
    int yCursor;

    //more crappy serialization
    inline friend std::ostream &operator<<(std::ostream &os, InputList const &toPrint)
    {
        os << toPrint.left << " ";
        os << toPrint.right << " ";
        os << toPrint.up << " ";
        os << toPrint.down << " ";
        os << toPrint.portalUsed << " ";
        os << toPrint.abilityUsed << " ";
        os << static_cast<int>(toPrint.abilityCursor) << " ";
        os << toPrint.timeCursor << " ";
        os << toPrint.xCursor << " ";
        os << toPrint.yCursor;
        return os;
    }
    inline friend std::istream &operator>>(std::istream &is, InputList &toRead)
    {
        is >> toRead.left;
        is >> toRead.right;
        is >> toRead.up;
        is >> toRead.down;
        is >> toRead.portalUsed;
        is >> toRead.abilityUsed;
        int abilityCursor;
        is >> abilityCursor;
        toRead.abilityCursor = static_cast<Ability>(abilityCursor);
        is >> toRead.timeCursor;
        is >> toRead.xCursor;
        is >> toRead.yCursor;
        return is;
    }
};
}
BOOST_CLASS_VERSION(hg::InputList, 2)
#endif //HG_INPUT_LIST_H
