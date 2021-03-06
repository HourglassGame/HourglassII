#ifndef HG_BOX_GLITZ_ADDER_H
#define HG_BOX_GLITZ_ADDER_H
#include "Glitz.h"
#include "mt/std/vector"
#include "mt/std/string"
#include "mp/std/vector"
#include "vector2.h"
#include "ImageGlitz.h"
#include "multi_thread_allocator.h"
#include "RectangleGlitz.h"
#include "mt/std/memory"
namespace hg {
class BoxGlitzAdder final {
public:
    BoxGlitzAdder(
        mt::std::vector<Glitz> &forwardsGlitz,
        mt::std::vector<Glitz> &reverseGlitz,
        mp::std::vector<GlitzPersister> &persistentGlitz) :
    forwardsGlitz(&forwardsGlitz), 
    reverseGlitz(&reverseGlitz),
    persistentGlitz(&persistentGlitz)
    {}
    //Adds the glitz that would be appropriate for a box
    //with the given characteristics
    void addGlitzForBox(
        vector2<int> const &position,
        int size,
        TimeDirection timeDirection) const
    {
        Glitz sameDirectionGlitz(mt::std::make_unique<ImageGlitz>(
                500, mt::std::string("global.box"),
                position.x, position.y,
                size, size));

        Glitz oppositeDirectionGlitz(mt::std::make_unique<ImageGlitz>(
                500, mt::std::string("global.box_r"),
                position.x, position.y,
                size, size));
        
        forwardsGlitz->push_back(
            timeDirection == TimeDirection::FORWARDS ? sameDirectionGlitz : oppositeDirectionGlitz);
        
        reverseGlitz->push_back(
            timeDirection == TimeDirection::REVERSE ? sameDirectionGlitz : oppositeDirectionGlitz);
    }

    void addDeathGlitz(
        int x,
        int y,
        int size,
        TimeDirection timeDirection) const
    {
        persistentGlitz->push_back(
            GlitzPersister(
                mt::std::make_unique<StaticGlitzPersister>(
                    Glitz(
                        mt::std::make_unique<RectangleGlitz>(
                                1500,
                                x,
                                y,
                                size,
                                size,
                                timeDirection == TimeDirection::FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
                    Glitz(
                        mt::std::make_unique<RectangleGlitz>(
                                1500,
                                x,
                                y,
                                size,
                                size,
                                timeDirection == TimeDirection::REVERSE ? 0xFF000000u : 0x00FFFF00u)),
                    60,
                    timeDirection)));
    }
private:
    mt::std::vector<Glitz> *forwardsGlitz;
    mt::std::vector<Glitz> *reverseGlitz;
    mp::std::vector<GlitzPersister> *persistentGlitz;
};
}
#endif //HG_BOX_GLITZ_ADDER_H
