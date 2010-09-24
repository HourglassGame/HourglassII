#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H

#include "Button.h"
#include "PlatformDestination.h"
#include <vector>

namespace hg
{
    class TriggerSystem
    {
        public:
            TriggerSystem(::std::vector<int> platformsTriggeredByButton, 
                          unsigned int triggerCount,
                          unsigned int platformCount,
                          ::std::vector<PlatformDestination> onDestinations,
                          ::std::vector<PlatformDestination> offDestinations);

            ::std::vector<PlatformDestination>& getPlatformDestinations(
                const ::std::vector<Button>& oldButtonList, 
                std::vector<PlatformDestination>& platformDesinations
            ) const;

        private:
            ::std::vector<int> platformsTriggeredByButton;
            unsigned int triggerCount;
            unsigned int platformCount;
            ::std::vector<PlatformDestination> onDestinations;
            ::std::vector<PlatformDestination> offDestinations;
    };
}
#endif //HG_TRIGGER_SYSTEM_H
