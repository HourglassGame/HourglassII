#ifndef HG_PLATFORM_DESTINATION_H
#define HG_PLATFORM_DESTINATION_H

namespace hg
{
struct PositionAndVelocity {
    PositionAndVelocity(int nPosition, int nVelocity) :
    position(nPosition), velocity(nVelocity) {}
    int position;
    int velocity;
};
class PlatformDestinationComponent {
public:
    PlatformDestinationComponent(
        int desiredPosition,
        int maxSpeed,
        int acceleration,
        int deceleration);   
    PositionAndVelocity solvePDEquation(PositionAndVelocity currentPositionAndVelocity) const;
private:
    int desiredPosition_;
    int maxSpeed_;
    int acceleration_;
    int deceleration_;
};
class PlatformDestination
{
public:
    PlatformDestination(
        PlatformDestinationComponent xComponent,
        PlatformDestinationComponent yComponent) :
            xComponent_(xComponent),
            yComponent_(yComponent)
    {
    }

    const PlatformDestinationComponent &getXDestination() const {
        return xComponent_;
    }
    const PlatformDestinationComponent &getYDestination() const {
        return yComponent_;
    }
private:
    PlatformDestinationComponent xComponent_;
    PlatformDestinationComponent yComponent_;
};

struct PlatformDestinationPair {
    PlatformDestinationPair(
        const PlatformDestination &newOnDestination,
        const PlatformDestination &newOffDestination) :
    onDestination(newOnDestination),
    offDestination(newOffDestination) {}
    PlatformDestination onDestination;
    PlatformDestination offDestination;
};
}

#endif //HG_PLATFORM_DESTINATION_H
