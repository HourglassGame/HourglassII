#include "GameDisplayHelpers.h"

#include "sfRenderTargetCanvas.h"

#include "hg/GlobalConst.h"
#include "VulkanEngine.h"
#include <gsl/gsl>

#include <sstream>
#include "hg/Util/Maths.h"
namespace hg {



void PlayAudioGlitz(
	hg::mt::std::vector<hg::Glitz> const &glitz,
	AudioPlayingState &audioPlayingState,
	AudioGlitzManager &audioGlitzManager,
	int const guyIndex)
{
	hg::sfRenderTargetAudioCanvas canvas(audioPlayingState, audioGlitzManager);

	hg::LayeredCanvas layeredCanvas(canvas);
	for (hg::Glitz const &particularGlitz : glitz)
	{
		particularGlitz.display(layeredCanvas, guyIndex);
	}
	hg::Flusher flusher(layeredCanvas.getFlusher());
	flusher.partialFlush(std::numeric_limits<int>::max());

	canvas.flushFrame();
}

struct CompareIndicies {
	template<typename IndexableType>
	bool operator()(IndexableType const &l, IndexableType const &r) {
		return l.getIndex() < r.getIndex();
	}
};

hg::FrameID mousePosToFrameID(GLFWWindow &windowglfw, hg::TimeEngine const &timeEngine) {
	double mouseX, mouseY;
	glfwGetCursorPos(windowglfw.w, &mouseX, &mouseY);
	int width, height;
	glfwGetWindowSize(windowglfw.w, &width, &height);

	int const timelineLength = timeEngine.getTimelineLength();
	int const mouseFrame = static_cast<int>((mouseX - width*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X))*1. / (width*((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X))*timelineLength);
	return hg::FrameID(clamp(0, mouseFrame, timelineLength-1), hg::UniverseID(timelineLength));
}

std::size_t mousePosToGuyIndex(GLFWWindow &windowglfw, hg::TimeEngine const &timeEngine) {
	double mouseX, mouseY;
	glfwGetCursorPos(windowglfw.w, &mouseX, &mouseY);
	int width, height;
	glfwGetWindowSize(windowglfw.w, &width, &height);

	int const timelineLength = timeEngine.getTimelineLength();
	float timelineOffset = static_cast<float>(width*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
	float timelineWidth = static_cast<float>(width*((1.f - hg::UI_DIVIDE_X) - 2.f*hg::TIMELINE_PAD_X));
	std::size_t const guyFrames{timeEngine.getGuyFrames().size()};
	int personalTimelineWidth{gsl::narrow_cast<int>((timeEngine.getReplayData().size() > 0) ? std::min(timelineWidth, timelineWidth*static_cast<float>(guyFrames) / static_cast<float>(timeEngine.getTimelineLength())) : timelineWidth)};

	int mouseGuyIndex = static_cast<int>(static_cast<float>(guyFrames)*(mouseX - timelineOffset)*1. / personalTimelineWidth);
	if (mouseGuyIndex < 0)
	{
		mouseGuyIndex = 0;
	}
	else if (mouseGuyIndex > guyFrames - 1)
	{
		mouseGuyIndex = gsl::narrow<int>(guyFrames) - 2;
	}
	return static_cast<std::size_t>(mouseGuyIndex);
}

}
