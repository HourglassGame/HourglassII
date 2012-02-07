#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H

#include "Geometry.h"

#include <boost/move/move.hpp>
#include <boost/multi_array.hpp>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
    	Wall(Wall const& o) :
            segmentSize_(o.segmentSize_),
            wallmap_(o.wallmap_)
        {}
    	Wall& operator=(BOOST_COPY_ASSIGN_REF(Wall) o)
        {
            segmentSize_ = o.segmentSize_;
            wallmap_ = o.wallmap_;
            return *this;
        }
    	Wall(BOOST_RV_REF(Wall) o) :
    		segmentSize_(boost::move(o.segmentSize_)),
    		wallmap_(boost::move(o.wallmap_))
    	{
    	}
    	Wall& operator=(BOOST_RV_REF(Wall) o)
    	{
    		segmentSize_ = boost::move(o.segmentSize_);
            //Note that boost::multi_array does not have a no-throw swap or
            //move constructor.
    		wallmap_ = boost::move(o.wallmap_);
    		return *this;
    	}
        Wall(
            int segmentSize,
            boost::multi_array<bool, 2> const& wallmap) :
                segmentSize_(segmentSize),
                wallmap_(wallmap)
        {
        }

        //Inlined for performance. Benefit needs to be tested.
        bool at(int x, int y) const
        {
        	if (x < 0 || y < 0) return true;

        	unsigned aX(x/segmentSize_);
			unsigned aY(y/segmentSize_);

			return aX >= wallmap_.size() || aY >= wallmap_[aX].size() || wallmap_[aX][aY];
        }
        /*
        bool atIndex(int x, int y) const
        {
            if (x < 0 || y < 0) return true;
            
            return x >= wallmap_.size() || aY >= wallmap_[aX].size() || wallmap_[x][y];
        }*/

        Rect<int> transformBounds(Rect<int> const& input)
        {
            int minx(input.x/segmentSize_);
            if(input.x < 0) minx -= 1;
            int maxx((input.x + input.w) / segmentSize_);
            if(input.x + input.w < 0) maxx -= 1;
            int miny(input.y/segmentSize_);
            if(input.y < 0) miny -= 1;
            int maxy((input.y + input.h) / segmentSize_);
            if(input.y + input.h < 0) maxy -= 1;
            
            return Rect<int>(minx, miny, maxx - minx, maxy - miny);
        }

        bool inTopLeftTriangle(int x, int y) const
        {
        	return (x - (x/segmentSize_)*segmentSize_ + y - (y/segmentSize_)*segmentSize_
    				 < segmentSize_);
        }

        bool inTopRightTriangle(int x, int y) const
        {
        	return x - (x/segmentSize_)*segmentSize_ > y - (y/segmentSize_)*segmentSize_;
        }

        int segmentSize() const{
            return segmentSize_;
        }
        int roomWidth() const{
        	return static_cast<int>(wallmap_.shape()[0] * segmentSize_);
		}
		int roomHeight() const{
			return static_cast<int>(wallmap_.shape()[1] * segmentSize_);
		}
    private:
        int segmentSize_;
        boost::multi_array<bool, 2> wallmap_;
        BOOST_COPYABLE_AND_MOVABLE(Wall)
    };
    inline void swap(Wall& l, Wall& r)
    {
        Wall temp(boost::move(l));
        l = boost::move(r);
        r = boost::move(temp);
    }
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment {
    	Environment(Environment const& o) :
            wall(o.wall),
            gravity(o.gravity)
        {
        }
    	Environment& operator=(BOOST_COPY_ASSIGN_REF(Environment) o)
        {
            wall = o.wall;
            gravity = o.gravity;
            return *this;
        }
    	Environment(BOOST_RV_REF(Environment) o) :
            wall(boost::move(o.wall)),
            gravity(boost::move(o.gravity))
        {}
    	Environment& operator=(BOOST_RV_REF(Environment) o) {
            wall = boost::move(o.wall);
            gravity = boost::move(o.gravity);
            return *this;
        }
        Environment(Wall const& nWall, int nGravity):
            wall(nWall),
            gravity(nGravity)
        {
        }
        Wall wall;
        int gravity;
    private:
        BOOST_COPYABLE_AND_MOVABLE(Environment)
    };
    inline void swap(Environment& l, Environment& r)
    {
        Environment temp(boost::move(l));
        l = boost::move(r);
        r = boost::move(temp);
    }
}

#endif //HG_ENVIRONMENT_H
