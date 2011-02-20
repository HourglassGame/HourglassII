#include "InputList.h"
namespace hg {
    InputList::InputList(bool nLeft,
                         bool nRight,
                         bool nUp,
                         bool nDown,
                         bool nUse,
                         Ability nAbility,
                         FrameID nFrameIdParams,
                         int nFrameIdParamCount) :
    left(nLeft),
    right(nRight),
    up(nUp),
    down(nDown),
    use(nUse),
    ability(nAbility),
    frameIdParams(nFrameIdParams),
    frameIdParamCount(nFrameIdParamCount)
    {
    }
}