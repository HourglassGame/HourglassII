#ifndef HG_BUTTON_H
#define HG_BUTTON_H
#include "TimeDirection.h"
namespace hg
{
    class Button
    {

    public:

        Button(int x, int y, int xspeed, int yspeed, int index, bool state, TimeDirection timeDirection, int pauseLevel);
        Button(const Button& other, TimeDirection timeDirection, int pauseLevel);
        ~Button();
        Button(const Button& other);
        Button& operator=(const Button& other);

        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getIndex() const {return data->index;}
        inline bool getState() const {return data->state;}
        inline TimeDirection getTimeDirection() const {return data->timeDirection;}
        inline int getPauseLevel() const {return data->pauseLevel;}

        bool operator==(const Button& other) const;
        bool operator!=(const Button& other) const;

        bool operator<(const Button& second) const;

    private:
        void decrementCount();

        struct Data;
        mutable int* referenceCount;
        Data* data;

        struct Data {
            Data(
                int nx,
                int ny,
                int nxspeed,
                int nyspeed,
                int nindex,
                bool nstate,
                TimeDirection ntimeDirection,
                int npauseLevel) :
            x(nx),
            y(ny),
            xspeed(nxspeed),
            yspeed(nyspeed),
            index(nindex),
            state(nstate),
            timeDirection(ntimeDirection),
            pauseLevel(npauseLevel)
            {
            }

            int x;
            int y;
            int xspeed;
            int yspeed;
            int index;
            bool state;
            TimeDirection timeDirection;
            int pauseLevel;
        };
    };
}
#endif //HG_BUTTON_H
