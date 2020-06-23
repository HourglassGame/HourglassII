#ifndef HG_BOX_H
#define HG_BOX_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "ConstPtr_of_fwd.h"
#include <tuple>
#include <ostream>
namespace hg {
enum class BoxType : int {
	NONE = 0,
	CRATE = 1,
	BOMB = 2,
	BALLOON = 3
};

class Box;
std::ostream &operator<<(std::ostream &str, Box const &b);
class Box final : boost::totally_ordered<Box>
{
public:
	explicit Box(int x, int y,
		int xspeed, int yspeed,
		int width,
		int height,
		BoxType boxType,
		int state,
		int illegalPortal,
		int arrivalBasis,
		TimeDirection timeDirection);
	explicit Box(Box const &o, TimeDirection timeDirection);

	int getX()      const { return x; }
	int getY()      const { return y; }
	int getXspeed() const { return xspeed; }
	int getYspeed() const { return yspeed; }
	int getWidth()  const { return width; }
	int getHeight() const { return height; }
	BoxType getBoxType()   const { return boxType; }
	int getState()         const { return state; }
	int getIllegalPortal() const { return illegalPortal; }
	int getArrivalBasis()  const { return arrivalBasis; }
	TimeDirection 
		getTimeDirection()    const { return timeDirection; }

	bool operator==(Box const &o) const;
	bool operator<(Box const &o) const;

private:
	int x;
	int y;
	int xspeed;
	int yspeed;
	int width;
	int height;
	BoxType boxType;

	int state;
	int illegalPortal;
	int arrivalBasis;

	TimeDirection timeDirection;
	
	auto comparison_tuple() const noexcept
	{
		return std::tie(
			x, y, xspeed, yspeed, width, height, boxType,
			state, illegalPortal, arrivalBasis, timeDirection);
	}
	//For debugging
	friend std::ostream &operator<<(std::ostream &str, Box const &b);
};

class BoxConstPtr final : boost::totally_ordered<BoxConstPtr>
{
public:
	explicit BoxConstPtr(Box const &box) : box_(&box) {}
	typedef Box base_type;
	Box const &get() const  { return *box_; }
	
	int getX()            const { return box_->getX(); }
	int getY()            const { return box_->getY(); }
	int getXspeed()       const { return box_->getXspeed(); }
	int getYspeed()       const { return box_->getYspeed(); }
	int getWidth()        const { return box_->getWidth(); }
	int getHeight()       const { return box_->getHeight(); }
	BoxType getBoxType()  const { return box_->getBoxType(); }
	int getState()        const { return box_->getState(); }
	int getIllegalPortal()const { return box_->getIllegalPortal(); }
	int getArrivalBasis() const { return box_->getArrivalBasis(); }
	TimeDirection 
		getTimeDirection()const { return box_->getTimeDirection(); }

	bool operator==(BoxConstPtr const &o) const { return *box_ == *o.box_; }
	bool operator< (BoxConstPtr const &o) const { return *box_ <  *o.box_; }

private:
	Box const *box_;
};
template<>
struct ConstPtr_of<Box> {
	typedef BoxConstPtr type;
};
}
#endif //HG_BOX_H
