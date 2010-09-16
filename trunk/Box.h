#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
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
            timeDirection(ntimeDirection)
            {
            }
            
            int x;
            int y;
            int xspeed;
            int yspeed;
            int size;
            
            TimeDirection timeDirection;
        };
    };
}
#endif //HG_BOX_H
