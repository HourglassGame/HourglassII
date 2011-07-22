#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H
#include <boost/multi_array.hpp>
#include <boost/move/move.hpp>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
        Wall(
            int segmentSize,
            boost::multi_array<bool, 2> const& wallmap);
            
        Wall(Wall const& other);
        Wall& operator=(BOOST_COPY_ASSIGN_REF(Wall) other);
        Wall(BOOST_RV_REF(Wall) other);
        Wall& operator=(BOOST_RV_REF(Wall) other);
        bool at(int x, int y) const;
        int segmentSize() const;
    private:
        int segmentSize_;
        boost::multi_array<bool, 2> wallmap_;
        BOOST_COPYABLE_AND_MOVABLE(Wall)
    };
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment {
        Environment(Environment const& other);
        Environment& operator=(BOOST_COPY_ASSIGN_REF(Environment) other);
        Environment(BOOST_RV_REF(Environment) other);
        Environment& operator=(BOOST_RV_REF(Environment) other);
        Environment(BOOST_RV_REF(Wall) nWall, int nGravity);
        Wall wall;
        int gravity;
    private:
        BOOST_COPYABLE_AND_MOVABLE(Environment)
    };
}

#endif //HG_ENVIRONMENT_H

