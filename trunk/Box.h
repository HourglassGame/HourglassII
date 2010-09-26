#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
namespace hg {
    class Box
    {
    public:
        Box(int x, int y, int xspeed, int yspeed, int size, TimeDirection timeDirection, int pauseLevel);
        ~Box();
        Box(const Box& other);
        Box& operator=(const Box& other);

        int getX() const {return data->x;}
        int getY() const {return data->y;}
        int getXspeed() const {return data->xspeed;}
        int getYspeed() const {return data->yspeed;}
        int getSize() const {return data->size;}
        TimeDirection getTimeDirection() const {return data->timeDirection;}
        int getPauseLevel() const {return data->pauseLevel;}

        bool operator==(const Box& other) const;
        bool operator!=(const Box& other) const;

        bool operator<(const Box& second) const;

    private:
        void decrementCount();

        struct Data;
        mutable int* referenceCount;
        Data* data;

        struct Data {
            Data(int nx,
                 int ny,
                 int nxspeed,
                 int nyspeed,
                 int nsize,
                 TimeDirection ntimeDirection,
                 int npauseLevel) :
            x(nx),
            y(ny),
            xspeed(nxspeed),
            yspeed(nyspeed),
            size(nsize),
            timeDirection(ntimeDirection),
            pauseLevel(npauseLevel)
            {
            }

            int x;
            int y;
            int xspeed;
            int yspeed;
            int size;

            TimeDirection timeDirection;
            int pauseLevel;
        };
    };
}
#endif //HG_BOX_H
