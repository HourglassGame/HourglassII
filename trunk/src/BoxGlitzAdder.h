#ifndef HG_BOX_GLITZ_ADDER_H
#define HG_BOX_GLITZ_ADDER_H
#include "Glitz.h"
#include "mt/std/vector"
#include "vector2.h"
#include "ImageGlitz.h"
#include "multi_thread_allocator.h"
namespace hg {
class BoxGlitzAdder {
public:
    BoxGlitzAdder(
        mt::std::vector<Glitz> &forwardsGlitz,
        mt::std::vector<Glitz> &reverseGlitz) :
    forwardsGlitz(&forwardsGlitz), reverseGlitz(&reverseGlitz)
    {}
    //Adds the glitz that would be appropriate for a box
    //with the given characteristics
    void addGlitzForBox(
        vector2<int> const &position,
        int size,
        TimeDirection timeDirection) const
    {
        Glitz sameDirectionGlitz(
            multi_thread_new<ImageGlitz>(
                500, std::string("global.box"),
                position.x, position.y,
                size, size));
    
        Glitz oppositeDirectionGlitz(
            multi_thread_new<ImageGlitz>(
                500, std::string("global.box_r"),
                position.x, position.y,
                size, size));
        
        forwardsGlitz->push_back(
            timeDirection == FORWARDS ? sameDirectionGlitz : oppositeDirectionGlitz);
        
        reverseGlitz->push_back(
            timeDirection == REVERSE ? sameDirectionGlitz : oppositeDirectionGlitz);
    }
private:
    mt::std::vector<Glitz> *forwardsGlitz;
    mt::std::vector<Glitz> *reverseGlitz;
};
}
#endif //HG_BOX_GLITZ_ADDER_H
