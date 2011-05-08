#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H
#include <boost/multi_array.hpp>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall {
    public:
        Wall(
            int segmentSize,
            const boost::multi_array<bool, 2>& wallmap);
        bool at(int x, int y) const;
        int segmentSize() const;
    private:
        int segmentSize_;
        boost::multi_array<bool, 2> wallmap_;
    };
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment {
        Environment(const Wall& nWall, int nGravity);
        Wall wall;
        int gravity;
    };
}

#endif //HG_ENVIRONMENT_H

