#ifndef HG_BOX_GLITZ_ADDER_H
#define HG_BOX_GLITZ_ADDER_H
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
		int width, int height,
		BoxType boxType, int state,
		TimeDirection timeDirection,
		int drawOrder) const
	{
		int x = position.a;
		int y = position.b;
		int w = width;
		int h = height;
		if (boxType == BoxType::BALLOON) {
			x = x - w/5;
			w = w * 7 / 5;
			h = h * 7 / 5;
		}
		//Glitz sameDirectionGlitzDebug(mt::std::make_unique<ImageGlitz>(
		//		drawOrder + static_cast<int>(boxType), getBoxImage(BoxType::CRATE, TimeDirection::FORWARDS),
		//		position.a, position.b, width, height));
		//forwardsGlitz->push_back(sameDirectionGlitzDebug);

		Glitz sameDirectionGlitz(mt::std::make_unique<ImageGlitz>(
				drawOrder + static_cast<int>(boxType), getBoxImage(boxType, TimeDirection::FORWARDS),
				x, y, w, h));

		Glitz oppositeDirectionGlitz(mt::std::make_unique<ImageGlitz>(
				drawOrder + static_cast<int>(boxType), getBoxImage(boxType, TimeDirection::REVERSE),
				x, y, w, h));
		
		forwardsGlitz->push_back(
			timeDirection == TimeDirection::FORWARDS ? sameDirectionGlitz : oppositeDirectionGlitz);
		
		reverseGlitz->push_back(
			timeDirection == TimeDirection::REVERSE ? sameDirectionGlitz : oppositeDirectionGlitz);

		if (boxType == BoxType::BOMB && state > 0) {
			Glitz stateText = Glitz(mt::std::make_unique<TextGlitz>(
				drawOrder + static_cast<int>(boxType),
				formatTime(state),
				x + width / 5,
				y + height / 5,
				width / 2,
				asPackedColour(0, 0, 0)));

			forwardsGlitz->push_back(stateText);
			reverseGlitz->push_back(stateText);
		}
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
