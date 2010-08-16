#ifndef HG_GUY_H
#define HG_GUY_H

#include "TimeDirection.h"
#include <boost/functional/hash.hpp>
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
            unsigned int nRelativeIndex,
            int nSubimage);
        
        ~Guy();
        Guy(const Guy& other);
        Guy& operator=(const Guy& other);
        
        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getWidth() const {return data->width;}
        inline int getHeight() const {return data->height;}
        
        inline bool getBoxCarrying() const {return data->boxCarrying;}
        inline int getBoxCarrySize() const {return data->boxCarrySize;}
        inline TimeDirection getBoxCarryDirection() const {return data->boxCarryDirection;}
        
        inline TimeDirection getTimeDirection() const {return data->timeDirection;}
        inline unsigned int getRelativeIndex() const {return data->relativeIndex;}
        inline int getSubimage() const {return data->subimage;}
        
        const static int animationLength = 13;
        
        bool operator!=(const Guy& other) const;
        bool operator==(const Guy& other) const;
        
        bool operator<(const Guy& second) const;
    private:
        void decrementCount();
        
        struct Data;
        mutable int* referenceCount;
        Data* data;
        
        friend ::std::size_t hash_value(const Guy& toHash);
        
        struct Data {
            Data(int nx,
                 int ny,
                 int nxspeed,
                 int nyspeed,
                 int nwidth,
                 int nheight,
                 
                 bool nboxCarrying,
                 int nboxCarrySize,
                 TimeDirection nboxCarryDirection,
                 
                 TimeDirection ntimeDirection,
                 unsigned int nrelativeIndex,

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
            
            subimage(nsubimage),
            
            hashValue(0)
            {
                //TODO - move to initializer list if possible.
                hashValue = hash_this();
            }
            
            int x;
            int y;
            int xspeed;
            int yspeed;
            int width;
            int height;
            
            bool boxCarrying;
            int boxCarrySize;
            TimeDirection boxCarryDirection;
            
            TimeDirection timeDirection;
            unsigned int relativeIndex;
            
            int subimage;
            
            ::std::size_t hashValue;
            
            ::std::size_t hash_this()
            {
                ::std::size_t seed(0);
                boost::hash_combine(seed, x);
                boost::hash_combine(seed, y);
                boost::hash_combine(seed, xspeed);
                boost::hash_combine(seed, yspeed);
                boost::hash_combine(seed, width);
                boost::hash_combine(seed, height);
                boost::hash_combine(seed, boxCarrying);
                boost::hash_combine(seed, boxCarrySize);
                boost::hash_combine(seed, boxCarryDirection);
                boost::hash_combine(seed, timeDirection);
                boost::hash_combine(seed, relativeIndex);
                boost::hash_combine(seed, subimage);
                return seed;
            }
        };
    };
    ::std::size_t hash_value(const Guy& toHash);
}
#endif //HG_GUY_H
