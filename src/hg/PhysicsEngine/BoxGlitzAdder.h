#ifndef HG_BOX_GLITZ_ADDER_H
#define HG_BOX_GLITZ_ADDER_H
#include "hg/PhysicsEngine/GlitzAdderUtil.h"
#include "hg/TimeEngine/Glitz/Glitz.h"
#include "hg/TimeEngine/Glitz/ImageGlitz.h"
#include "hg/TimeEngine/Glitz/RectangleGlitz.h"
#include "hg/Util/multi_thread_allocator.h"
#include "hg/mt/std/memory"
#include "hg/mt/std/vector"
#include "hg/mt/std/string"
#include "hg/mp/std/vector"
namespace hg {
class BoxGlitzAdder final {
public:
	explicit BoxGlitzAdder(
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
		vec2<int> const &position,
		int width, int height, BoxType boxType,
		TimeDirection timeDirection) const
	{
		Glitz sameDirectionGlitz(mt::std::make_unique<ImageGlitz>(
				500, getBoxImage(boxType, TimeDirection::FORWARDS),
				position.a, position.b,
				width, height));

		Glitz oppositeDirectionGlitz(mt::std::make_unique<ImageGlitz>(
				500, getBoxImage(boxType, TimeDirection::REVERSE),
				position.a, position.b,
				width, height));
		
		forwardsGlitz->push_back(
			timeDirection == TimeDirection::FORWARDS ? sameDirectionGlitz : oppositeDirectionGlitz);
		
		reverseGlitz->push_back(
			timeDirection == TimeDirection::REVERSE ? sameDirectionGlitz : oppositeDirectionGlitz);
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
};
}
#endif //HG_BOX_GLITZ_ADDER_H
