#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H
#include <boost/move/move.hpp>
#include <boost/multi_array.hpp>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
    	Wall(Wall const& other) :
            segmentSize_(other.segmentSize_),
            wallmap_(other.wallmap_)
        {}
    	Wall& operator=(BOOST_COPY_ASSIGN_REF(Wall) other)
        {
            segmentSize_ = other.segmentSize_;
            wallmap_ = other.wallmap_;
            return *this;
        }
    	Wall(BOOST_RV_REF(Wall) other) :
    		segmentSize_(boost::move(other.segmentSize_)),
    		wallmap_(boost::move(other.wallmap_))
    	{
    	}
    	Wall& operator=(BOOST_RV_REF(Wall) other)
    	{
    		segmentSize_ = boost::move(other.segmentSize_);
            //Note that boost::multi_array does not have a no-throw swap or
            //move constructor.
    		wallmap_ = boost::move(other.wallmap_);
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
    	Environment(Environment const& other) :
            wall(other.wall),
            gravity(other.gravity)
        {
        }
    	Environment& operator=(BOOST_COPY_ASSIGN_REF(Environment) other)
        {
            wall = other.wall;
            gravity = other.gravity;
            return *this;
        }
    	Environment(BOOST_RV_REF(Environment) other) :
            wall(boost::move(other.wall)),
            gravity(boost::move(other.gravity))
        {}
    	Environment& operator=(BOOST_RV_REF(Environment) other) {
            wall = boost::move(other.wall);
            gravity = boost::move(other.gravity);
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
