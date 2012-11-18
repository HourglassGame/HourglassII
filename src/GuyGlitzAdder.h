#ifndef HG_GUY_GLITZ_ADDER_H
#define HG_GUY_GLITZ_ADDER_H
#include "Glitz.h"
#include "mt/std/vector"
#include "vector2.h"
#include "multi_thread_allocator.h"
#include "RectangleGlitz.h"
namespace hg {
class GuyGlitzAdder {
public:
    GuyGlitzAdder(
        mt::std::vector<Glitz>::type& forwardsGlitz,
        mt::std::vector<Glitz>::type& reverseGlitz) :
    forwardsGlitz_(&forwardsGlitz), reverseGlitz_(&reverseGlitz)
    {}
    //Adds the glitz that would be appropriate for a box
    //with the given characteristics
    void addGlitzForGuy(
        vector2<int> const& position,
        vector2<int> const& velocity,
        vector2<int> const& size,
        TimeDirection timeDirection,
        bool facing,
        bool boxCarrying,
        int boxCarrySize,
        TimeDirection boxCarryDirection) const
    {
        //Forwards View
        {
            struct PNC {
                PNC(int nx, int ny, unsigned ncolour):
                    x(nx), y(ny), colour(ncolour) {}
                int x; int y; unsigned colour;
            } const pnc = 
                timeDirection == FORWARDS ?
                    PNC(position.x, position.y, 0x96960000u) :
                    PNC(position.x - velocity.x, position.y - velocity.y, 0x00009600u);
            
            int const left(pnc.x);
            int const top(pnc.y);
            int const halfwidth(size.x/2);
            int const halfheight(size.y/2);
            int const hmid(pnc.x+halfwidth);
            
            forwardsGlitz_->push_back(Glitz(multi_thread_new<RectangleGlitz>(600, left, top, size.x, size.y, pnc.colour)));
            forwardsGlitz_->push_back(
                facing ?
                    Glitz(multi_thread_new<RectangleGlitz>(600, hmid, top, halfwidth, halfheight, 0x32323200u)) :
                    Glitz(multi_thread_new<RectangleGlitz>(600, left, top, halfwidth, halfheight, 0x32323200u)));
            
            if (boxCarrying)
            {
                forwardsGlitz_->push_back(
                    Glitz(
                        multi_thread_new<RectangleGlitz>(
                            600,
                            hmid - boxCarrySize/2,
                            top - boxCarrySize,
                            boxCarrySize,
                            boxCarrySize,
                            boxCarryDirection == FORWARDS ? 
                              0x96009600u : 0x00960000u)));
            }
        }
        //Reverse View
        {
            struct PNC {
                PNC(int nx, int ny, unsigned ncolour):
                    x(nx), y(ny), colour(ncolour) {}
                int x; int y; unsigned colour;
            } const pnc = 
                timeDirection == REVERSE ?
                    PNC(position.x, position.y, 0x96960000u) :
                    PNC(position.x - velocity.x, position.y - velocity.y, 0x00009600u);

            int const left(pnc.x);
            int const top(pnc.y);
            int const halfwidth(size.x/2);
            int const halfheight(size.y/2);
            int const hmid(pnc.x+halfwidth);

            reverseGlitz_->push_back(Glitz(multi_thread_new<RectangleGlitz>(600, left, top, size.x, size.y, pnc.colour)));
            reverseGlitz_->push_back(
                facing ?
                    Glitz(multi_thread_new<RectangleGlitz>(600, hmid, top, halfwidth, halfheight, 0x32323200u)) :
                    Glitz(multi_thread_new<RectangleGlitz>(600, left, top, halfwidth, halfheight, 0x32323200u)));

            if (boxCarrying)
            {
                reverseGlitz_->push_back(
                    Glitz(
                        multi_thread_new<RectangleGlitz>(
                            600,
                            hmid - boxCarrySize/2,
                            top - boxCarrySize,
                            boxCarrySize,
                            boxCarrySize,
                            boxCarryDirection == REVERSE ? 
                              0x96009600u : 0x00960000u)));
            }
        }
    }
private:
    mt::std::vector<Glitz>::type* forwardsGlitz_;
    mt::std::vector<Glitz>::type* reverseGlitz_;
};
}
#endif //HG_GUY_GLITZ_ADDER_H
