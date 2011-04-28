#ifndef HG_PLATFORM_DESTINATION_H
#define HG_PLATFORM_DESTINATION_H

namespace hg
{
class PlatformDestination
{
public:
    PlatformDestination(
        int nx,
        int nxspeed,
        int nxaccel,
        int nxdeccel,
        int ny,
        int nyspeed,
        int nyaccel,
        int nydeccel) :
            x(nx),
            xspeed(nxspeed),
            xaccel(nxaccel),
            xdeccel(nxdeccel),

            y(ny),
            yspeed(nyspeed),
            yaccel(nyaccel),
            ydeccel(nydeccel)
    {
    }

    int getX() const {
        return x;
    }
    int getXspeed() const {
        return xspeed;
    }
    int getXaccel() const {
        return xaccel;
    }
    int getXdeccel() const {
        return xdeccel;
    }

    int getY() const {
        return y;
    }
    int getYspeed() const {
        return yspeed;
    }
    int getYaccel() const {
        return yaccel;
    }
    int getYdeccel() const {
        return ydeccel;
    }

private:
    int x;
    int xspeed;
    int xaccel;
    int xdeccel;

    int y;
    int yspeed;
    int yaccel;
    int ydeccel;
};
}

#endif //HG_PLATFORM_DESTINATION_H
