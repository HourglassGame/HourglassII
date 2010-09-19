#include "TriggerSystem.h"
#include <iostream>

using namespace ::hg;


TriggerSystem::TriggerSystem(::std::vector<int> nPlatformsTriggeredByButton, int nTriggerCount, int nPlatformCount,
                          ::std::vector<PlatformDestination> nonDestinations, ::std::vector<PlatformDestination> noffDestinations) :
platformsTriggeredByButton(nPlatformsTriggeredByButton),
triggerCount(nTriggerCount),
platformCount(nPlatformCount),
onDestinations(nonDestinations),
offDestinations(noffDestinations)
{
}

::std::vector<PlatformDestination>& TriggerSystem::getPlatformDestinations(const ::std::vector<Button>& oldButtonList, std::vector<PlatformDestination>& platformDesinations) const
{

    std::vector<bool> triggerInputs(triggerCount, false);
    for (unsigned int i = 0; i < oldButtonList.size(); ++i)
    {
        triggerInputs[oldButtonList[i].getIndex()] = oldButtonList[i].getState();
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
    return platformDesinations;
}
