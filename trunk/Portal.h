#ifndef HG_PORTAL_H
#define HG_PORTAL_H
#include "TimeDirection.h"
namespace hg
{
    class Portal
    {

    public:

        Portal(int x, int y, int xspeed, int yspeed, int width, int height, int index, TimeDirection timeDirection, int pauseLevel, int charges, bool active,
               int xDestination, int yDestination, int destinationIndex, int timeDestination, bool relativeTime);
        Portal(const Portal& other, TimeDirection timeDirection, int pauseLevel);
        ~Portal();
        Portal(const Portal& other);
        Portal& operator=(const Portal& other);

        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getWidth() const {return data->width;}
        inline int getHeight() const {return data->height;}
        inline int getIndex() const {return data->index;}
        inline TimeDirection getTimeDirection() const {return data->timeDirection;}
        inline int getPauseLevel() const {return data->pauseLevel;}
        inline int getCharges() const {return data->charges;}
        inline bool getActive() const {return data->active;}
        inline int getXdestination() const {return data->xDestination;}
        inline int getYdestination() const {return data->yDestination;}
        inline int getDestinationIndex() const {return data->destinationIndex;}
        inline int getTimeDestination() const {return data->timeDestination;}
        inline bool getRelativeTime() const {return data->relativeTime;}

        bool operator==(const Portal& other) const;
        bool operator!=(const Portal& other) const;

        bool operator<(const Portal& second) const;

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
                TimeDirection nTimeDirection,
                int npauseLevel,
                int ncharges,
                bool nactive,
                int nxDestination,
                int nyDestination,
                int ndestinationIndex,
                int ntimeDestination,
                bool nrelativeTime
                ) :
            x(nx),
            y(ny),
            xspeed(nXspeed),
            yspeed(nYspeed),
            width(nWidth),
            height(nHeight),
            index(nIndex),
            timeDirection(nTimeDirection),
            pauseLevel(npauseLevel),
            charges(ncharges),
            active(nactive),
            xDestination(nxDestination),
            yDestination(nyDestination),
            destinationIndex(ndestinationIndex),
            timeDestination(ntimeDestination),
            relativeTime(nrelativeTime)
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
            int pauseLevel;
            int charges;
            bool active;
            int xDestination;
            int yDestination;
            int destinationIndex;
            int timeDestination;
            bool relativeTime;
        };
    };
}
#endif //HG_PLATFORM_H
