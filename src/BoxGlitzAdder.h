#ifndef HG_BOX_GLITZ_ADDER_H
#define HG_BOX_GLITZ_ADDER_H
#include "Glitz.h"
#include "mt/std/vector"
#include "vector2.h"
#include "RectangleGlitz.h"
#include "multi_thread_allocator.h"
namespace hg {
class BoxGlitzAdder {
public:
    BoxGlitzAdder(
        mt::std::vector<Glitz>::type& forwardsGlitz,
        mt::std::vector<Glitz>::type& reverseGlitz) :
    forwardsGlitz_(&forwardsGlitz), reverseGlitz_(&reverseGlitz)
    {}
    //Adds the glitz that would be appropriate for a box
    //with the given characteristics
    void addGlitzForBox(
        vector2<int> const& position,
        int size,
        TimeDirection timeDirection) const
    {
        Glitz sameDirectionGlitz(
            multi_thread_new<RectangleGlitz>(
                500,
                position.x, position.y,
                size, size,
                0xFF00FF00u));
    
        Glitz oppositeDirectionGlitz(
            multi_thread_new<RectangleGlitz>(
                500,
                position.x, position.y,
                size, size,
                0x00FF0000u));
        
        forwardsGlitz_->push_back(
            timeDirection == FORWARDS ? sameDirectionGlitz : oppositeDirectionGlitz);
        
        reverseGlitz_->push_back(
            timeDirection == REVERSE ? sameDirectionGlitz : oppositeDirectionGlitz);
    }
private:
    mt::std::vector<Glitz>::type *forwardsGlitz_;
    mt::std::vector<Glitz>::type *reverseGlitz_;
};
}
#endif //HG_BOX_GLITZ_ADDER_H
