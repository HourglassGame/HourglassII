#include "Environment.h"
#include <cassert>

namespace hg {
    Wall::Wall(
        int segmentSize,
        const boost::multi_array<bool, 2>& wallmap) :
            segmentSize_(segmentSize),
            wallmap_(wallmap)
    {
        assert(segmentSize > 0);
    }
    bool Wall::at(int x, int y) const
    {
        if (x < 0 || y < 0)
        {
            return true;
        }

        unsigned aX(x/segmentSize_);
        unsigned aY(y/segmentSize_);

        if (aX < wallmap_.size() && aY < wallmap_[aX].size())
        {
            return wallmap_[aX][aY];
        }
        else
        {
            return true;
        }
    }
    int Wall::segmentSize() const {
        return segmentSize_;
    }
        
    Environment::Environment(const Wall& nWall, int nGravity) :
    wall(nWall), gravity(nGravity) {}
}
