#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H
#include <boost/multi_array.hpp>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
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
        	return (x - (x/segmentSize_)*segmentSize_ > y - (y/segmentSize_)*segmentSize_);
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
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment {
        Environment(Wall const& nWall, int nGravity):
            wall(nWall),
            gravity(nGravity)
        {
        }
        Wall wall;
        int gravity;
    };
}

#endif //HG_ENVIRONMENT_H
