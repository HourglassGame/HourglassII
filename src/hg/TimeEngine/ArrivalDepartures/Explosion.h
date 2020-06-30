#ifndef HG_EXPLOSION_H
#define HG_EXPLOSION_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
#include <tuple>
#include <ostream>
namespace hg {

class Explosion;
std::ostream &operator<<(std::ostream &str, Explosion const &e);
class Explosion final : boost::totally_ordered<Explosion>
{
public:
	explicit Explosion(int x, int y,
		int width, int height,
		int radius, int radiusMax,
		int radiusGrow,
		TimeDirection timeDirection);
	explicit Explosion(Explosion const &o, TimeDirection timeDirection);

	int getX()      const { return x; }
	int getY()      const { return y; }
	int getWidth()  const { return width; }
	int getHeight() const { return height; }
	int getRadius() const { return radius; }
	int getRadiusMax()  const { return radiusMax; }
	int getRadiusGrow() const { return radiusGrow; }
	TimeDirection getTimeDirection() const { return timeDirection; }

	bool operator==(Explosion const &o) const;
	bool operator<(Explosion const &o) const;

private:
	int x;
	int y;
	int width;
	int height;
	int radius;
	int radiusMax;
	int radiusGrow;
	TimeDirection timeDirection;
	
	auto comparison_tuple() const noexcept
	{
		return std::tie(
			x, y, width, height, radius, radiusMax, radiusGrow, timeDirection);
	}
	//For debugging
	friend std::ostream &operator<<(std::ostream &str, Explosion const &e);
};

class ExplosionConstPtr final : boost::totally_ordered<ExplosionConstPtr>
{
public:
	explicit ExplosionConstPtr(Explosion const &explosion) : explosion_(&explosion) {}
	typedef Explosion base_type;
	Explosion const &get() const  { return *explosion_; }
	
	int getX()            const { return explosion_->getX(); }
	int getY()            const { return explosion_->getY(); }
	int getWidth()        const { return explosion_->getWidth(); }
	int getheight()       const { return explosion_->getHeight(); }
	int getRadius()       const { return explosion_->getRadius(); }
	int getRadiusMax()    const { return explosion_->getRadiusMax(); }
	int getRadiusGrow()   const { return explosion_->getRadiusGrow(); }
	TimeDirection getTimeDirection()const { return explosion_->getTimeDirection(); }

	bool operator==(ExplosionConstPtr const &o) const { return *explosion_ == *o.explosion_; }
	bool operator< (ExplosionConstPtr const &o) const { return *explosion_ <  *o.explosion_; }

private:
	Explosion const *explosion_;
};
template<>
struct ConstPtr_of<Explosion> {
	typedef ExplosionConstPtr type;
};
}
#endif //HG_EXPLOSION_H
