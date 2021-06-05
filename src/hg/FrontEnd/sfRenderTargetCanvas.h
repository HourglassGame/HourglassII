#ifndef HG_SF_RENDER_TARGET_CANVAS_H
#define HG_SF_RENDER_TARGET_CANVAS_H
#include "ResourceManager.h"
#include "Canvas.h"
#include <iostream>
namespace hg {

class sfRenderTargetAudioCanvas final : public Canvas
{
public:
	explicit sfRenderTargetAudioCanvas(
		AudioPlayingState &audioPlayingState,
		AudioGlitzManager &audioGlitzManager) :
	audioPlayingState(&audioPlayingState),
	audioGlitzManager(&audioGlitzManager),
	soundsToPlay()
	{}
	virtual void playSound(std::string const &key, int const n) override {
		soundsToPlay.push_back(AudioGlitzObject(key,n));
	}
	virtual void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
	{
		(void)x; (void)y; (void)width; (void)height; (void)colour;
	}
	virtual void drawCircle(float const x, float const y, float const radius, unsigned const colour) override
	{
		(void)x; (void)y; (void)radius; (void)colour;
	}
	virtual void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
	{
		(void)xa; (void)ya; (void)xb; (void)yb; (void)width; (void)colour;
	}
	virtual void drawText(std::string const &text, float const x, float const y, float const size, unsigned const colour) override
	{
		(void)text; (void)x; (void)y; (void)size; (void)colour;
	}
	virtual void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
	{
		(void)key; (void)x; (void)y; (void)width; (void)height;
	}
	virtual void flushFrame() override {
		audioPlayingState->runStep(audioGlitzManager->updatePlayingState(soundsToPlay));
	}
private:
	AudioPlayingState *audioPlayingState;
	AudioGlitzManager *audioGlitzManager;
	std::vector<AudioGlitzObject> soundsToPlay;
};

}
#endif //HG_SF_RENDER_TARGET_CANVAS_H
