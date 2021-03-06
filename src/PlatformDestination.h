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
    PositionAndVelocity solvePlatformDestinationEquation(PositionAndVelocity currentPositionAndVelocity) const;
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

    PlatformDestinationComponent const &getXDestination() const {
        return xComponent_;
    }
    PlatformDestinationComponent const &getYDestination() const {
        return yComponent_;
    }
private:
    PlatformDestinationComponent xComponent_;
    PlatformDestinationComponent yComponent_;
};

struct PlatformDestinationPair {
    PlatformDestination onDestination;
    PlatformDestination offDestination;
};
}

#endif //HG_PLATFORM_DESTINATION_H
