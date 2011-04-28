#include "TriggerSystem.h"

namespace hg {
TriggerSystem::TriggerSystem(std::vector<int> nPlatformsTriggeredByButton,
                            unsigned int nButtonCount,
                            unsigned int nTriggerCount,
                            unsigned int nPlatformCount,
                            std::vector<PlatformDestination> nonDestinations,
                            std::vector<PlatformDestination> noffDestinations) :
platformsTriggeredByButton(nPlatformsTriggeredByButton),
buttonCount(nButtonCount),
triggerCount(nTriggerCount),
platformCount(nPlatformCount),
onDestinations(nonDestinations),
offDestinations(noffDestinations)
{
}

void TriggerSystem::getPlatformDestinations(const std::vector<char>& buttonState,
                                            std::vector<PlatformDestination>& platformDesinations) const
{

    std::vector<bool> triggerInputs(triggerCount, false);
    for (unsigned int i = 0; i < buttonCount; ++i)
    {
        triggerInputs[i] = buttonState[i];
    }

    platformDesinations.reserve(platformCount);

    for (unsigned int i = 0; i < triggerCount; ++i)
    {
        if (triggerInputs[i])
        {
            platformDesinations.push_back(onDestinations[i]);

        }
        else
        {
            platformDesinations.push_back(offDestinations[i]);
        }
    }
}
}//namespace hg
