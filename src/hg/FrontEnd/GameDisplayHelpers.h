#ifndef HG_GAME_DISPLAY_HELPERS_H
#define HG_GAME_DISPLAY_HELPERS_H
#include "hg/mt/std/vector"
#include "hg/TimeEngine/Glitz/Glitz.h"
#include "hg/PhysicsEngine/Environment.h"
#include "ResourceManager.h"
#include "hg/TimeEngine/TimeEngine.h"
#include <boost/range/adaptor/reversed.hpp>
#include "AudioGlitzManager.h"
#include "VulkanEngine.h"
#include "GLFWWindow.h"

namespace hg {
	void PlayAudioGlitz(
		hg::mt::std::vector<hg::Glitz> const &glitz,
		AudioPlayingState &audioPlayingState,
		AudioGlitzManager &audioGlitzManager,
		int const guyIndex);
#if 0
	template<typename BidirectionalGuyRange>
	hg::GuyOutputInfo const &findCurrentGuy(BidirectionalGuyRange const &guyRange, std::size_t index)
	{
		for (GuyOutputInfo const &guyInfo : guyRange)
		{
			if (guyInfo.getIndex() == index)
			{
				return guyInfo;
			}
		}
		assert(false);
		return *boost::begin(guyRange | boost::adaptors::reversed);
	}
#endif
	template<typename BidirectionalGuyRange>
	bool const &guyExists(BidirectionalGuyRange const &guyRange, std::size_t index)
	{
		for (GuyOutputInfo const &guyInfo : guyRange)
		{
			if (guyInfo.getIndex() == index)
			{
				return true;
			}
		}
		return false;
	}

	hg::mt::std::vector<hg::Glitz> const &getGlitzForDirection(
		hg::FrameView const &view, hg::TimeDirection timeDirection);
	hg::FrameID mousePosToFrameID(GLFWWindow &windowglfw, hg::TimeEngine const &timeEngine);
	std::size_t mousePosToGuyIndex(GLFWWindow &windowglfw, hg::TimeEngine const &timeEngine);
}

#endif //HG_GAME_DISPLAY_HELPERS_H
