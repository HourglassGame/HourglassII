#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H

#include "Geometry.h"
#include <utility>
#include "multi_array.h"
#include <string>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
        Wall(
            int segmentSize,
            hg::multi_array<bool, 2> const &wallmap,
            std::string const &tilesetName) :
                segmentSize_(segmentSize),
                wallmap_(wallmap),
                tilesetName_(tilesetName)
        {
        }

        bool at(int x, int y) const
        {
        	if (x < 0 || y < 0) return true;

        	unsigned const aX(x/segmentSize_);
			unsigned const aY(y/segmentSize_);

			return aX >= wallmap_.extents()[0] || aY >= wallmap_.extents()[1] || wallmap_[aX][aY];
        }
        
        bool atIndex(int x, int y) const
        {
            if (x < 0 || y < 0) return true;

            unsigned const aX(x);
			unsigned const aY(y);

			return aX >= wallmap_.extents()[0] || aY >= wallmap_.extents()[1] || wallmap_[aX][aY];
        }

        Rect<int> transformBounds(Rect<int> const &input)
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

        int segmentSize() const {
            return segmentSize_;
        }
        int roomWidth() const {
        	return static_cast<int>(wallmap_.extents()[0] * segmentSize_);
		}
		int roomHeight() const {
			return static_cast<int>(wallmap_.extents()[1] * segmentSize_);
		}
        std::string const &tilesetName() const {
            return tilesetName_;
        }
    private:
        int segmentSize_;
        hg::multi_array<bool, 2> wallmap_;
        std::string tilesetName_;
    };
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment {
        Wall wall;
        int gravity;
    };
    inline void swap(Environment &l, Environment &r) noexcept
    {
        Environment temp(std::move(l));
        l = std::move(r);
        r = std::move(temp);
    }
}

#endif //HG_ENVIRONMENT_H
