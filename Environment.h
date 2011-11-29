#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H
#include "move.h"
#include <boost/multi_array.hpp>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
    	Wall(Wall const& other) :
            segmentSize_(other.segmentSize_),
            wallmap_(other.wallmap_)
        {}
    	Wall& operator=(Wall const& other)
        {
            segmentSize_ = other.segmentSize_;
            wallmap_ = other.wallmap_;
            return *this;
        }
    	Wall(Wall&& other) :
    		segmentSize_(other.segmentSize_),
    		wallmap_(other.wallmap_)
    	{
    	}
    	Wall& operator=(Wall&& other)
    	{
    		segmentSize_ = other.segmentSize_;
    		wallmap_ = other.wallmap_;
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
        	if (x < 0 || y < 0)
			{
				return true;
			}
        	unsigned aX(x/segmentSize_);
			unsigned aY(y/segmentSize_);

			return (aX >= wallmap_.size() || aY >= wallmap_[aX].size()) || wallmap_[aX][aY];
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
    };
    inline void swap(Wall& l, Wall& r)
    {
        Wall temp(hg::move(l));
        l = hg::move(r);
        r = hg::move(temp);
    }
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment {
    	Environment(Environment&& other) :
            wall(hg::move(other.wall)),
            gravity(hg::move(other.gravity))
        {}
    	Environment& operator=(Environment&& other) {
            wall = hg::move(other.wall);
            gravity = hg::move(other.gravity);
            return *this;
        }
    	Environment(Environment const& other) :
            wall(other.wall),
            gravity(other.gravity)
        {
        }
    	Environment& operator=(Environment const& other)
        {
            wall = other.wall;
            gravity = other.gravity;
            return *this;
        }
        Environment(Wall const& nWall, int nGravity):
            wall(nWall),
            gravity(nGravity)
        {
        }
        Wall wall;
        int gravity;
    };
    inline void swap(Environment& l, Environment& r)
    {
        Environment temp(hg::move(l));
        l = hg::move(r);
        r = hg::move(temp);
    }
}

#endif //HG_ENVIRONMENT_H
