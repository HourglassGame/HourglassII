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
    TriggerSystem(std::vector<int> platformsTriggeredByButton,
                  unsigned int buttonCount,
                  unsigned int triggerCount,
                  unsigned int platformCount,
                  std::vector<PlatformDestination> onDestinations,
                  std::vector<PlatformDestination> offDestinations);

    void getPlatformDestinations(
        const std::vector<char>& buttonState,
        std::vector<PlatformDestination>& platformDesinations
    ) const;

    unsigned int getPlatformCount() const {
        return platformCount;
    };
    unsigned int getButtonCount() const {
        return buttonCount;
    };

private:
    std::vector<int> platformsTriggeredByButton;
    unsigned int buttonCount;
    unsigned int triggerCount;
    unsigned int platformCount;
    std::vector<PlatformDestination> onDestinations;
    std::vector<PlatformDestination> offDestinations;
};
}
#endif //HG_TRIGGER_SYSTEM_H
