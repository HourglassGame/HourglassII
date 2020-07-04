#ifndef HG_EXPLOSION_GLITZ_ADDER_H
#define HG_EXPLOSION_GLITZ_ADDER_H
#include "hg/GlobalConst.h"
#include "hg/PhysicsEngine/GlitzAdderUtil.h"
#include "hg/TimeEngine/Glitz/Glitz.h"
#include "hg/TimeEngine/Glitz/ImageGlitz.h"
#include "hg/TimeEngine/Glitz/RectangleGlitz.h"
#include "hg/TimeEngine/Glitz/CircleGlitz.h"
#include "hg/TimeEngine/Glitz/TextGlitz.h"
#include "hg/Util/multi_thread_allocator.h"
#include "hg/mt/std/memory"
#include "hg/mt/std/vector"
#include "hg/mt/std/string"
#include "hg/mp/std/vector"
namespace hg {
class ExplosionGlitzAdder final {
public:

	explicit ExplosionGlitzAdder(
		mt::std::vector<Glitz> &forwardsGlitz,
		mt::std::vector<Glitz> &reverseGlitz,
		mp::std::vector<GlitzPersister> &persistentGlitz) :
	forwardsGlitz(&forwardsGlitz),
	reverseGlitz(&reverseGlitz),
	persistentGlitz(&persistentGlitz)
	{}
	
	void addExplosionGlitz(
		int x,
		int y,
		int width,
		int height,
		int radius,
		TimeDirection timeDirection) const
	{
		int drawX = x + width/2;
		int drawY = y + height/2;
		int drawRadius = radius + (width + height) / 4;
		
		Glitz sameDirectionGlitz(mt::std::make_unique<CircleGlitz>(
			1500,
			drawX,
			drawY,
			drawRadius,
			timeDirection == TimeDirection::FORWARDS ? 0xFF000000u : 0x00FFFF00u));

		Glitz oppositeDirectionGlitz(mt::std::make_unique<CircleGlitz>(
			1500,
			drawX,
			drawY,
			drawRadius,
			timeDirection == TimeDirection::REVERSE ? 0xFF000000u : 0x00FFFF00u));
		
		forwardsGlitz->push_back(
			timeDirection == TimeDirection::FORWARDS ? sameDirectionGlitz : oppositeDirectionGlitz);
		
		reverseGlitz->push_back(
			timeDirection == TimeDirection::REVERSE ? sameDirectionGlitz : oppositeDirectionGlitz);
	}

	void addExplosionFinalGlitz(
		int x,
		int y,
		int width,
		int height,
		int radius,
		TimeDirection timeDirection) const
	{
		int drawX = x + width/2;
		int drawY = y + height/2;
		int drawRadius = radius + (width + height) / 4;
		
		persistentGlitz->push_back(
			GlitzPersister(
				mt::std::make_unique<StaticGlitzPersister>(
					Glitz(
						mt::std::make_unique<CircleGlitz>(
								1500,
								drawX,
								drawY,
								drawRadius,
								timeDirection == TimeDirection::FORWARDS ? 0xFF000000u : 0x00FFFF00u)),
					Glitz(
						mt::std::make_unique<CircleGlitz>(
								1500,
								drawX,
								drawY,
								drawRadius,
								timeDirection == TimeDirection::REVERSE ? 0xFF000000u : 0x00FFFF00u)),
					4,
					timeDirection)));
		persistentGlitz->push_back(
			GlitzPersister(
				mt::std::make_unique<StaticGlitzPersister>(
					Glitz(
						mt::std::make_unique<CircleGlitz>(
								100,
								drawX,
								drawY,
								drawRadius,
								0x88888800u)),
					Glitz(
						mt::std::make_unique<CircleGlitz>(
								100,
								drawX,
								drawY,
								drawRadius,
								0x88888800u)),
					-1,
					timeDirection)));
	}

private:
	mt::std::vector<Glitz> *forwardsGlitz;
	mt::std::vector<Glitz> *reverseGlitz;
	mp::std::vector<GlitzPersister> *persistentGlitz;
};
}
#endif //HG_EXPLOSION_GLITZ_ADDER_H
