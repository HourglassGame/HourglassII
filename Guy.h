#ifndef HG_GUY_H
#define HG_GUY_H
#include "TimeDirection.h"
#include <cstring>
namespace hg {
    class Guy
    {
    public:
        Guy(int nX,
            int nY,
            int nXspeed,
            int nYspeed,
            int nWidth,
            int nHeight,
            bool nSupported,
            bool nBoxCarrying,
            int nBoxCarrySize,
            hg::TimeDirection nBoxCarryDirection,
            hg::TimeDirection nTimeDirection,
            size_t nRelativeIndex,
            int nSubimage);

        ~Guy();
        Guy(const Guy& other);
        Guy& operator=(const Guy& other);

        int getX() const {return data->x;}
        int getY() const {return data->y;}
        int getXspeed() const {return data->xspeed;}
        int getYspeed() const {return data->yspeed;}
        int getWidth() const {return data->width;}
        int getHeight() const {return data->height;}
        int getSupported() const {return data->supported;}

        bool getBoxCarrying() const {return data->boxCarrying;}
        int getBoxCarrySize() const {return data->boxCarrySize;}
        TimeDirection getBoxCarryDirection() const {return data->boxCarryDirection;}

        TimeDirection getTimeDirection() const {return data->timeDirection;}
        size_t getRelativeIndex() const {return data->relativeIndex;}
        int getSubimage() const {return data->subimage;}

        const static int animationLength = 13;

        bool operator!=(const Guy& other) const;
        bool operator==(const Guy& other) const;

        bool operator<(const Guy& second) const;
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
                 int nwidth,
                 int nheight,
                 int nsupported,

                 bool nboxCarrying,
                 int nboxCarrySize,
                 TimeDirection nboxCarryDirection,

                 TimeDirection ntimeDirection,
                 size_t nrelativeIndex,

                 int nsubimage) :
            x(nx),
            y(ny),
            xspeed(nxspeed),
            yspeed(nyspeed),
            width(nwidth),
            height(nheight),
            supported(nsupported),

            boxCarrying(nboxCarrying),
            boxCarrySize(nboxCarrySize),
            boxCarryDirection(nboxCarryDirection),

            timeDirection(ntimeDirection),
            relativeIndex(nrelativeIndex),

            subimage(nsubimage)
            {
            }

            int x;
            int y;
            int xspeed;
            int yspeed;
            int width;
            int height;
            bool supported;

            bool boxCarrying;
            int boxCarrySize;
            TimeDirection boxCarryDirection;

            TimeDirection timeDirection;
            size_t relativeIndex;

            int subimage;
        };
    };
}
#endif //HG_GUY_H