#ifndef HG_PLATFORM_H
#define HG_PLATFORM_H
#include "TimeDirection.h"
namespace hg
{
    class Platform
    {

    public:

        Platform(int x, int y, int xspeed, int yspeed, int width, int height, int index, TimeDirection timeDirection);
        ~Platform();
        Platform(const Platform& other);
        Platform& operator=(const Platform& other);

        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getWidth() const {return data->width;}
        inline int getHeight() const {return data->height;}
        inline int getIndex() const {return data->index;}
        inline TimeDirection getTimeDirection() const {return data->timeDirection;}

        bool operator==(const Platform& other) const;
        bool operator!=(const Platform& other) const;

        bool operator<(const Platform& second) const;

    private:
        void decrementCount();

        struct Data;
        mutable int* referenceCount;
        Data* data;

        struct Data {
            Data(
                int nx,
                int ny,
                int nXspeed,
                int nYspeed,
                int nWidth,
                int nHeight,
                int nIndex,
                TimeDirection nTimeDirection) :
            x(nx),
            y(ny),
            xspeed(nXspeed),
            yspeed(nYspeed),
            width(nWidth),
            height(nHeight),
            index(nIndex),
            timeDirection(nTimeDirection)
            {
            }

            int x;
            int y;
            int xspeed;
            int yspeed;
            int width;
            int height;
            int index;
            TimeDirection timeDirection;
        };
    };
}
#endif //HG_PLATFORM_H
