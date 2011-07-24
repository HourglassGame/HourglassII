#include "Environment.h"
#include <cassert>

namespace hg {
Wall::Wall(
    int segmentSize,
    boost::multi_array<bool, 2> const& wallmap) :
        segmentSize_(segmentSize),
        wallmap_(wallmap)
{
}

Wall::Wall(Wall const& other) :
        segmentSize_(other.segmentSize_),
        wallmap_(other.wallmap_)
{
}

Wall& Wall::operator=(BOOST_COPY_ASSIGN_REF(Wall) other)
{
    segmentSize_ = other.segmentSize_;
    wallmap_ = other.wallmap_;
    return *this;
}

Wall::Wall(BOOST_RV_REF(Wall) other) :
        segmentSize_(other.segmentSize_),
        wallmap_(other.wallmap_)
{
}

Wall& Wall::operator=(BOOST_RV_REF(Wall) other)
{
    segmentSize_ = other.segmentSize_;
    wallmap_ = other.wallmap_;
    return *this;
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
Environment::Environment(BOOST_RV_REF(Wall) nWall, int nGravity) :
    wall(boost::move(nWall)),
    gravity(nGravity)
{
}

Environment::Environment(Environment const& other) :
    wall(other.wall),
    gravity(other.gravity)
{
}

Environment& Environment::operator=(BOOST_COPY_ASSIGN_REF(Environment) other)
{
    wall = other.wall;
    gravity = other.gravity;
    return *this;
}

Environment::Environment(BOOST_RV_REF(Environment) other) :
    wall(boost::move(other.wall)),
    gravity(other.gravity)
{
}

Environment& Environment::operator=(BOOST_RV_REF(Environment) other)
{
    wall = boost::move(other.wall);
    gravity = other.gravity;
    return *this;
}
} //namespace hg
