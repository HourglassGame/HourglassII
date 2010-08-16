#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
#include <boost/functional/hash.hpp>
namespace hg {
    class Box
    {
    public:
        Box(int x, int y, int xspeed, int yspeed, int size, TimeDirection timeDirection);
        ~Box();
        Box(const Box& other);
        Box& operator=(const Box& other);
        
        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getSize() const {return data->size;}
        inline TimeDirection getTimeDirection() const {return data->timeDirection;}
        
        bool operator==(const Box& other) const;
        bool operator!=(const Box& other) const;
        
        bool operator<(const Box& second) const;
        
    private:
        void decrementCount();
        
        struct Data;
        mutable int* referenceCount;
        Data* data;
        friend ::std::size_t hash_value(const Box& toHash);
        struct Data {
            Data(int nx,
                 int ny,
                 int nxspeed,
                 int nyspeed,
                 int nsize,
                 TimeDirection ntimeDirection) :
            x(nx),
            y(ny),
            xspeed(nxspeed),
            yspeed(nyspeed),
            size(nsize),
            timeDirection(ntimeDirection),
            hashValue(0)
            {
                hashValue = hash_this();
            }
            
            int x;
            int y;
            int xspeed;
            int yspeed;
            int size;
            
            TimeDirection timeDirection;
            
            ::std::size_t hashValue;
            
            ::std::size_t hash_this()
            {
                ::std::size_t seed(0);
                boost::hash_combine(seed, x);
                boost::hash_combine(seed, y);
                boost::hash_combine(seed, xspeed);
                boost::hash_combine(seed, yspeed);
                boost::hash_combine(seed, size);
                boost::hash_combine(seed, timeDirection);
                return seed;
            }
        };
    };
    ::std::size_t hash_value(const Box& toHash);
}
#endif //HG_BOX_H
