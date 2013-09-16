#ifndef HG_GUY_GLITZ_ADDER_H
#define HG_GUY_GLITZ_ADDER_H
#include "Glitz.h"
#include "mt/std/vector"
#include "vector2.h"
#include "multi_thread_allocator.h"
#include "RectangleGlitz.h"
#include "LineGlitz.h"
namespace hg {

inline void addCurrentGuyArrow(
    mt::std::vector<Glitz>::type& glitzList, int hmid, int top, int halfwidth, vector2<int> size)
{
    int tipx = hmid;
    int tipy = top - 400;
    int width = 200;
    glitzList.push_back(
        Glitz(
            multi_thread_new<LineGlitz>(
                650,
                tipx,
                top - size.y/2 - 400,
                tipx,
                tipy,
                width,
                0xFF000000u)));
    
    glitzList.push_back(
        Glitz(
            multi_thread_new<LineGlitz>(
                650,
                tipx+halfwidth/2,
                top - size.y/4 - 400,
                hmid,
                tipy+200,
                width,
                0xFF000000u)));
    
    glitzList.push_back(
        Glitz(
            multi_thread_new<LineGlitz>(
                650,
                tipx-halfwidth/2,
                top - size.y/4 - 400,
                hmid,
                tipy+200,
                width,
                0xFF000000u)));
}

class GuyGlitzAdder {
public:
    GuyGlitzAdder(
        mt::std::vector<Glitz>::type& forwardsGlitz,
        mt::std::vector<Glitz>::type& reverseGlitz,
		mt::std::vector<GlitzPersister>::type& persistentGlitz) :
    forwardsGlitz(&forwardsGlitz),
	reverseGlitz(&reverseGlitz),
	persistentGlitz(&persistentGlitz)
    {}
    //Adds the glitz that would be appropriate for a guy
    //with the given characteristics
    void addGlitzForGuy(
        vector2<int> const& position,
        vector2<int> const& size,
        TimeDirection timeDirection,
        bool facing,
        bool boxCarrying,
        int boxCarrySize,
        TimeDirection boxCarryDirection,
        bool currentGuy) const
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
                    PNC(position.x, position.y, 0x00009600u);
            
            int const left(pnc.x);
            int const top(pnc.y);
            int const halfwidth(size.x/2);
            //int const halfheight(size.y/2);
            int const hmid(pnc.x+halfwidth);
            
            forwardsGlitz->push_back(Glitz(multi_thread_new<ImageGlitz>(
                600,
                facing ?
                    (timeDirection == FORWARDS ? "global.rhino_right_stop" : "global.rhino_right_stop_r") :
                    (timeDirection == FORWARDS ? "global.rhino_left_stop" : "global.rhino_left_stop_r"),
                left, top, size.x, size.y)));
            
            if (boxCarrying)
            {
                forwardsGlitz->push_back(
                    Glitz(
                        multi_thread_new<ImageGlitz>(
                            600,
                            boxCarryDirection == FORWARDS ? 
                              "global.box" : "global.box_r",
                            hmid - boxCarrySize/2,
                            top - boxCarrySize,
                            boxCarrySize,
                            boxCarrySize)));
            }
            
            if (currentGuy) addCurrentGuyArrow(*forwardsGlitz, hmid, top, halfwidth, size);
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
                    PNC(position.x, position.y, 0x00009600u);

            int const left(pnc.x);
            int const top(pnc.y);
            int const halfwidth(size.x/2);
            //int const halfheight(size.y/2);
            int const hmid(pnc.x+halfwidth);

            reverseGlitz->push_back(Glitz(multi_thread_new<ImageGlitz>(
                600,
                facing ?
                    (timeDirection == REVERSE ? "global.rhino_right_stop" : "global.rhino_right_stop_r") :
                    (timeDirection == REVERSE ? "global.rhino_left_stop" : "global.rhino_left_stop_r"),
                left, top, size.x, size.y)));
            if (boxCarrying)
            {
                reverseGlitz->push_back(
                    Glitz(
                        multi_thread_new<ImageGlitz>(
                            600,
                            boxCarryDirection == REVERSE ? 
                              "global.box" : "global.box_r",
                            hmid - boxCarrySize/2,
                            top - boxCarrySize,
                            boxCarrySize,
                            boxCarrySize)));
            }
            if (currentGuy) addCurrentGuyArrow(*reverseGlitz, hmid, top, halfwidth, size);
        }
    }
	
	void addLaserGlitz(
        int x1,
		int y1,
		int x2,
		int y2,
        int xAim,
        int yAim,
		TimeDirection timeDirection) const
	{
		int width = 100;
		persistentGlitz->push_back(
			GlitzPersister(
				Glitz(
					multi_thread_new<LineGlitz>(
						1500,
						x1,
						y1,
						x2,
						y2,
						width,
						timeDirection == FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
				Glitz(
					multi_thread_new<LineGlitz>(
						1500,
						x1,
						y1,
						x2,
						y2,
						width,
						timeDirection == REVERSE ? 0xFF000000u : 0x00FFFF00u)),
				60,
				timeDirection));
		persistentGlitz->push_back(
			GlitzPersister(
				Glitz(
					multi_thread_new<RectangleGlitz>(
						1500, 
						xAim-200,
						yAim-200,
						400, 
						400,
						timeDirection == FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
				Glitz(
					multi_thread_new<RectangleGlitz>(
						1500, 
						xAim-200,
						yAim-200,
						400, 
						400,
						timeDirection == REVERSE ? 0xFF000000u : 0x00FFFF00u)),
				60,
				timeDirection));
	}
	
	void addDeathGlitz(
		int x,
		int y,
		int width,
		int height,
		TimeDirection timeDirection) const
	{
		persistentGlitz->push_back(
			GlitzPersister(
				Glitz(
					multi_thread_new<RectangleGlitz>(
						1500,
						x,
						y,
						width,
						height,
						timeDirection == FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
				Glitz(
					multi_thread_new<RectangleGlitz>(
						1500,
						x,
						y,
						width,
						height,
						timeDirection == REVERSE ? 0xFF000000u : 0x00FFFF00u)),
				60,
				timeDirection));
	}
	
private:
    mt::std::vector<Glitz>::type *forwardsGlitz;
    mt::std::vector<Glitz>::type *reverseGlitz;
	mt::std::vector<GlitzPersister>::type *persistentGlitz;
};
}
#endif //HG_GUY_GLITZ_ADDER_H
