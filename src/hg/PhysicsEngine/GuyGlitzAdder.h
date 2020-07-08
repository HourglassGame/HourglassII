#ifndef HG_GUY_GLITZ_ADDER_H
#define HG_GUY_GLITZ_ADDER_H
#include "BoxGlitzAdder.h"
#include "hg/PhysicsEngine/GlitzAdderUtil.h"
#include "hg/TimeEngine/Glitz/RectangleGlitz.h"
#include "hg/TimeEngine/Glitz/CircleGlitz.h"
#include "hg/TimeEngine/Glitz/LineGlitz.h"
#include "hg/TimeEngine/Glitz/Glitz.h"
#include "hg/Util/multi_thread_allocator.h"

#include "hg/mt/std/vector"
#include "hg/mp/std/vector"
#include "hg/mt/std/memory"
namespace hg {

inline void addCurrentGuyArrow(
	mt::std::vector<Glitz> &glitzList, int index, int hmid, int top, int halfwidth, vec2<int> size)
{
	int tipx = hmid;
	int tipy = top - 400;
	int width = 200;
	glitzList.push_back(
		Glitz(mt::std::make_unique<LineGlitz>(
				650,
				tipx,
				top - size.b/2 - 400,
				tipx,
				tipy,
				width,
				0xFF000000u),
			index));
	
	glitzList.push_back(
		Glitz(mt::std::make_unique<LineGlitz>(
				650,
				tipx+halfwidth/2,
				top - size.b/4 - 400,
				hmid,
				tipy+200,
				width,
				0xFF000000u),
			index));
	
	glitzList.push_back(
		Glitz(mt::std::make_unique<LineGlitz>(
				650,
				tipx-halfwidth/2,
				top - size.b/4 - 400,
				hmid,
				tipy+200,
				width,
				0xFF000000u),
			index));
}

class GuyGlitzAdder final {
public:
	GuyGlitzAdder(
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		mp::std::vector<GlitzPersister> &persistentGlitz,
		BoxGlitzAdder const &boxGlitzAdder) :
	forwardsGlitz(&forwardsGlitz),
	reverseGlitz(&reverseGlitz),
	persistentGlitz(&persistentGlitz),
	boxGlitzAdder(boxGlitzAdder)
	{}
	//Adds the glitz that would be appropriate for a guy
	//with the given characteristics
	void addGlitzForGuy(
		vec2<int> const &position,
		vec2<int> const &size,
		TimeDirection timeDirection,
		FacingDirection facing,
		BoxType boxCarrying,
		int boxCarryWidth,
		int boxCarryHeight,
		int boxCarryState,
		TimeDirection boxCarryDirection,
		bool paused,
		std::size_t guyIndex,
		bool justPickedUpBox) const
	{
		//Forwards View
		{
			struct PNC {
				PNC(int nx, int ny, unsigned ncolour):
					x(nx), y(ny), colour(ncolour) {}
				int x; int y; unsigned colour;
			} const pnc = 
				timeDirection == TimeDirection::FORWARDS ?
					PNC(position.a, position.b, 0x96960000u) :
					PNC(position.a, position.b, 0x00009600u);

			int const left(pnc.x);
			int const top(pnc.y);
			int const halfwidth(size.a/2);
			//int const halfheight(size.y/2);
			int const hmid(pnc.x+halfwidth);

			forwardsGlitz->push_back(Glitz(mt::std::make_unique<ImageGlitz>(
				600,
				facing == FacingDirection::RIGHT ?
				(timeDirection == TimeDirection::FORWARDS ? "global.rhino_right_stop" : "global.rhino_right_stop_r") :
				(timeDirection == TimeDirection::FORWARDS ? "global.rhino_left_stop" : "global.rhino_left_stop_r"),
				left, top, size.a, size.b),
				paused ? static_cast<int>(guyIndex) : -1));

			if (boxCarrying != BoxType::NONE)
			{


				int x = hmid - boxCarryWidth/2;
				int y = top - boxCarryHeight;
				int w = boxCarryWidth;
				int h = boxCarryHeight;

				boxGlitzAdder.addGlitzForBox(
					vec2<int>{x, y}, w, h,
						boxCarrying, boxCarryState,
						boxCarryDirection, 600,
						paused ? static_cast<int>(guyIndex) : -1);
			}
			
			addCurrentGuyArrow(*forwardsGlitz, static_cast<int>(guyIndex), hmid, top, halfwidth, size);
		}
		//Reverse View
		{
			struct PNC {
				PNC(int nx, int ny, unsigned ncolour):
					x(nx), y(ny), colour(ncolour) {}
				int x; int y; unsigned colour;
			} const pnc = 
				timeDirection == TimeDirection::REVERSE ?
					PNC(position.a, position.b, 0x96960000u) :
					PNC(position.a, position.b, 0x00009600u);

			int const left(pnc.x);
			int const top(pnc.y);
			int const halfwidth(size.a/2);
			//int const halfheight(size.y/2);
			int const hmid(pnc.x+halfwidth);

			reverseGlitz->push_back(Glitz(mt::std::make_unique<ImageGlitz>(
				600,
				facing == FacingDirection::RIGHT ?
					(timeDirection == TimeDirection::REVERSE ? "global.rhino_right_stop" : "global.rhino_right_stop_r") :
					(timeDirection == TimeDirection::REVERSE ? "global.rhino_left_stop" : "global.rhino_left_stop_r"),
				left, top, size.a, size.b),
				paused ? static_cast<int>(guyIndex) : -1));
			if (boxCarrying != BoxType::NONE)
			{
				int x = hmid - boxCarryWidth/2;
				int y = top - boxCarryHeight;
				int w = boxCarryWidth;
				int h = boxCarryHeight;

				boxGlitzAdder.addGlitzForBox(
					vec2<int>{x, y}, w, h,
					boxCarrying, boxCarryState,
					boxCarryDirection, 600,
					paused ? static_cast<int>(guyIndex) : -1);
			}
			addCurrentGuyArrow(*reverseGlitz, static_cast<int>(guyIndex), hmid, top, halfwidth, size);
		}
		
		if (justPickedUpBox && !paused) {
			persistentGlitz->push_back(
				GlitzPersister(
					mt::std::make_unique<AudioGlitzPersister>(
						"global.box_pickup",
						16,
						timeDirection)));
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
				mt::std::make_unique<StaticGlitzPersister>(
					Glitz(mt::std::make_unique<LineGlitz>(
							1500,
							x1,
							y1,
							x2,
							y2,
							width,
							timeDirection == TimeDirection::FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
					Glitz(mt::std::make_unique<LineGlitz>(
							1500,
							x1,
							y1,
							x2,
							y2,
							width,
							timeDirection == TimeDirection::REVERSE ? 0xFF000000u : 0x00FFFF00u)),
					24,
					timeDirection)));
		persistentGlitz->push_back(
			GlitzPersister(
				mt::std::make_unique<StaticGlitzPersister>(
					Glitz(mt::std::make_unique<RectangleGlitz>(
							1500, 
							xAim-200,
							yAim-200,
							400, 
							400,
							timeDirection == TimeDirection::FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
					Glitz(mt::std::make_unique<RectangleGlitz>(
							1500, 
							xAim-200,
							yAim-200,
							400, 
							400,
							timeDirection == TimeDirection::REVERSE ? 0xFF000000u : 0x00FFFF00u)),
					24,
					timeDirection)));
		
		persistentGlitz->push_back(
			GlitzPersister(
				mt::std::make_unique<AudioGlitzPersister>(
					"global.laser_shoot",
					24,
					timeDirection)));
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
				mt::std::make_unique<StaticGlitzPersister>(
					Glitz(
						mt::std::make_unique<RectangleGlitz>(
							1500,
							x,
							y,
							width,
							height,
							timeDirection == TimeDirection::FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
					Glitz(
						mt::std::make_unique<RectangleGlitz>(
							1500,
							x,
							y,
							width,
							height,
							timeDirection == TimeDirection::REVERSE ? 0xFF000000u : 0x00FFFF00u)),
					60,
					timeDirection)));
	}
	
private:
	mt::std::vector<Glitz> *forwardsGlitz;
	mt::std::vector<Glitz> *reverseGlitz;
	mp::std::vector<GlitzPersister> *persistentGlitz;
	BoxGlitzAdder const &boxGlitzAdder;
};
}
#endif //HG_GUY_GLITZ_ADDER_H
