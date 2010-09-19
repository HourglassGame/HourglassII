#ifndef HG_PLATFORM_DESTINATION_H
#define HG_PLATFORM_DESTINATION_H

namespace hg
{

    class PlatformDestination
    {
        public:
            PlatformDestination(
                    int nx,
                    int ny,
                    int nspeed,
                    int naccel,
                    int ndeccel) :
            x(nx),
            y(ny),
            speed(nspeed),
            accel(naccel),
            deccel(ndeccel)
            {
            }

            int getX() const {return x;}
            int getY() const {return y;}
            int getSpeed() const {return speed;}
            int getAccel() const {return accel;}
            int getDeccel() const {return deccel;}

        private:
            int x;
            int y;
            int speed;
            int yspeed;
            int accel;
            int deccel;
    };
}

#endif //HG_PLATFORM_DESTINATION_H
