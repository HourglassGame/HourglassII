#ifndef HG_ENVIRONMENT_H
#define HG_ENVIRONMENT_H

#include <utility>
#include "hg/Util/multi_vector.h"
#include "hg/Util/Maths.h"
#include <string>
namespace hg {
    //stores the wall (the static environment over which the game is played)
    class Wall final {
    private:
        auto equality_tuple() const -> decltype(auto)
        {
            return std::tie(segmentSize_, wallmap_, tilesetName_);
        }
    public:
        Wall(
            int segmentSize,
            hg::multi_vector<bool, 2> wallmap,
            std::string tilesetName) :
                segmentSize_(segmentSize),
                wallmap_(std::move(wallmap)),
                tilesetName_(std::move(tilesetName))
        {
        }

        bool at(int x, int y) const noexcept
        {
            if (x < 0 || y < 0) return true;

            unsigned const aX(flooredDivision(x,segmentSize_));
            unsigned const aY(flooredDivision(y,segmentSize_));

            return aX >= wallmap_.size()[0] || aY >= wallmap_.size()[1] || wallmap_[aX][aY];
        }
        
        bool atIndex(int x, int y) const noexcept
        {
            if (x < 0 || y < 0) return true;

            unsigned const aX(x);
            unsigned const aY(y);

            return aX >= wallmap_.size()[0] || aY >= wallmap_.size()[1] || wallmap_[aX][aY];
        }

        Rect<int> transformBounds(Rect<int> const &input) const noexcept
        {
            int const minx(flooredDivision(input.x,segmentSize_));
            
            int const maxx(flooredDivision(input.x + input.w,segmentSize_));
            
            int const miny(flooredDivision(input.y,segmentSize_));
            int const maxy(flooredDivision(input.y + input.h, segmentSize_));
            return {minx, miny, maxx - minx, maxy - miny};
        }

        bool inTopLeftTriangle(int x, int y, int oldX, int oldY) const noexcept
        {
            return flooredModulo(x, segmentSize_) + oldX - x + flooredModulo(y, segmentSize_) + oldY - y < segmentSize_;
        }

        bool inTopRightTriangle(int x, int y, int oldX, int oldY) const noexcept
        {
            return flooredModulo(x, segmentSize_) + oldX - x > flooredModulo(y, segmentSize_) + oldY - y;
        }

        int segmentSize() const noexcept {
            return segmentSize_;
        }
        int roomWidth() const noexcept {
            return static_cast<int>(wallmap_.size()[0] * segmentSize_);
        }
        int roomHeight() const noexcept {
            return static_cast<int>(wallmap_.size()[1] * segmentSize_);
        }
        std::string const &tilesetName() const noexcept {
            return tilesetName_;
        }
        bool operator==(Wall const &o) const {
            return equality_tuple() == o.equality_tuple();
        }
    private:

        int segmentSize_;
        hg::multi_vector<bool, 2> wallmap_;
        std::string tilesetName_;
    };
    //Stores the physical attributes of the world
    //in which the game is played.
    //That is - the wall and gravity
    struct Environment final {
    private:
        auto equality_tuple() const -> decltype(auto)
        {
            return std::tie(wall, gravity);
        }
    public:
        Wall wall;
        int gravity;
        bool operator==(Environment const &o) const {
            return equality_tuple() == o.equality_tuple();
        }
    };
    inline void swap(Environment &l, Environment &r) noexcept
    {
        Environment temp(std::move(l));
        l = std::move(r);
        r = std::move(temp);
    }
}

#endif //HG_ENVIRONMENT_H
