#ifndef HG_GUY_H
#define HG_GUY_H

#define BOOST_SP_DISABLE_THREADS
#include <boost/shared_ptr.hpp>

#include "TimeDirection.h"
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
            bool nBoxCarrying, 
            int nBoxCarrySize,
            hg::TimeDirection nBoxCarryDirection,
            hg::TimeDirection nTimeDirection, 
            int nRelativeIndex,
            int nSubimage);
        
        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getWidth() const {return data->width;}
        inline int getHeight() const {return data->height;}
        
        inline bool getBoxCarrying() const {return data->boxCarrying;}
        inline int getBoxCarrySize() const {return data->boxCarrySize;}
        inline hg::TimeDirection getBoxCarryDirection() const {return data->boxCarryDirection;}
        
        inline hg::TimeDirection getTimeDirection() const {return data->timeDirection;}
        inline int getRelativeIndex() const {return data->relativeIndex;}
        inline int getSubimage() const {return data->subimage;}
        
        const static int animationLength = 13;
        
        bool operator!=(const Guy& other) const;
        bool operator==(const Guy& other) const;
        
        bool operator<(const Guy& second) const;
    private:
        struct Data;
        
        ::boost::shared_ptr<Data> data;
        
        struct Data {
            Data(int nx,
                 int ny,
                 int nxspeed,
                 int nyspeed,
                 int nwidth,
                 int nheight,
                 
                 bool nboxCarrying,
                 int nboxCarrySize,
                 hg::TimeDirection nboxCarryDirection,
                 
                 hg::TimeDirection ntimeDirection,
                 int nrelativeIndex,

                 int nsubimage) :
            x(nx),
            y(ny),
            xspeed(nxspeed),
            yspeed(nyspeed),
            width(nwidth),
            height(nheight),
            
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
            
            bool boxCarrying;
            int boxCarrySize;
            hg::TimeDirection boxCarryDirection;
            
            hg::TimeDirection timeDirection;
            int relativeIndex;
            
            int subimage;
        };
    };
}
#endif //HG_GUY_H
