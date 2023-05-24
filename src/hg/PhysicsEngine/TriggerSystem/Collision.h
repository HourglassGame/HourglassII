#ifndef HG_COLLISION_H
#define HG_COLLISION_H
#include "hg/TimeEngine/ArrivalDepartures/TimeDirection.h"
#include <ostream>
#include <tuple>
namespace hg
{
enum class CollisionType : int {
	PLATFORM = 0,
	LADDER = 1
};
std::ostream &operator<<(std::ostream &o, CollisionType collisionType);

class Collision;
std::ostream &operator<<(std::ostream &os, Collision const &toPrint);
class Collision final
{
public:
	Collision(
		int x, int y,
		int xspeed, int yspeed,
		int prevXspeed, int prevYspeed,
		int width, int height,
		bool functional,
		CollisionType collisionType,
		TimeDirection timeDirection) :
			x_(x), y_(y),
			xspeed_(xspeed), yspeed_(yspeed),
			prevXspeed_(prevXspeed), prevYspeed_(prevYspeed),
			width_(width), height_(height),
			functional_(functional),
			collisionType_(collisionType),
			timeDirection_(timeDirection) { }
	int getX()           const { return x_; }
	int getY()           const { return y_; }
	int getXspeed()      const { return xspeed_; }
	int getYspeed()      const { return yspeed_; }
	int getPrevXspeed()  const { return prevXspeed_; }
	int getPrevYspeed()  const { return prevYspeed_; }
	int getWidth()       const { return width_; }
	int getHeight()      const { return height_; }
	bool getFunctional() const { return functional_; }
	CollisionType getCollisionType()     const { return collisionType_; }
	TimeDirection 
		getTimeDirection() const { return timeDirection_; }
	friend std::ostream &operator<<(std::ostream &os, Collision const &toPrint)
	{
		#define HG_COLLISION_PRINT(obj) os << #obj << ": " << toPrint.obj << std::endl
		HG_COLLISION_PRINT(x_);
		HG_COLLISION_PRINT(y_);
		HG_COLLISION_PRINT(xspeed_);
		HG_COLLISION_PRINT(yspeed_);
		HG_COLLISION_PRINT(prevXspeed_);
		HG_COLLISION_PRINT(prevYspeed_);
		HG_COLLISION_PRINT(width_);
		HG_COLLISION_PRINT(height_);
		HG_COLLISION_PRINT(functional_);
		HG_COLLISION_PRINT(collisionType_);
		HG_COLLISION_PRINT(timeDirection_);
		#undef HG_COLLISION_PRINT
		return os;
	}
	bool operator==(Collision const& o) const = default;
private:
	int x_;
	int y_;
	int xspeed_;
	int yspeed_;
	int prevXspeed_;
	int prevYspeed_;
	int width_;
	int height_;
	bool functional_;
	CollisionType collisionType_;
	TimeDirection timeDirection_;
};

}
#endif //HG_COLLISION_H
