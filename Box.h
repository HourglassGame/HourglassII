#ifndef HG_BOX_H
#define HG_BOX_H

#define BOOST_SP_DISABLE_THREADS
#include <boost/shared_ptr.hpp>
#include "TimeDirection.h"
namespace hg {
<<<<<<< .mine
    class Box
    {
        
    public:
        Box(int x, int y, int xspeed, int yspeed, int size, hg::TimeDirection timeDirection);
        
        inline int getX() const {return data->x;}
        inline int getY() const {return data->y;}
        inline int getXspeed() const {return data->xspeed;}
        inline int getYspeed() const {return data->yspeed;}
        inline int getSize() const {return data->size;}
        inline hg::TimeDirection getTimeDirection() const {return data->timeDirection;}
        
        static const int normalSize = 1000;
        
        bool operator==(const Box& other) const;
        bool operator!=(const Box& other) const;
        
        bool operator<(const Box& second) const;
        
    private:
        struct Data;
        
        ::boost::shared_ptr<Data> data;
        
        struct Data {
            Data(int nx,
                 int ny,
                 int nxspeed,
                 int nyspeed,
                 int nsize,
                 hg::TimeDirection ntimeDirection) :
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
            
            hg::TimeDirection timeDirection;
        };
    };
=======
class Box
{

public:
    Box(int x, int y, int xspeed, int yspeed, int size, hg::TimeDirection timeDirection);

    inline int getX() const {return x;}
    inline int getY() const {return y;}
    inline int getXspeed() const {return xspeed;}
    inline int getYspeed() const {return yspeed;}
	inline int getSize() const {return size;}
	inline hg::TimeDirection getTimeDirection() const {return timeDirection;}

	static const int normalSize = 1000;

	bool operator==(const Box& other) const;
    bool operator!=(const Box& other) const;
    
    bool operator<(const Box& second) const;

private:
  
	int x;
	int y;
	int xspeed;
	int yspeed;
	int size;

    hg::TimeDirection timeDirection;
     
};
>>>>>>> .r93
}
#endif //HG_BOX_H
