#include "TimeDirection.h"
#include <cassert>
namespace hg {
TimeDirection &operator*=(TimeDirection &receiver, int multiple)
{
	assert(multiple == -1 || multiple == 1);
	receiver = static_cast<TimeDirection>(static_cast<int>(receiver)*multiple);
	return receiver;
}

TimeDirection &operator*=(TimeDirection &receiver, TimeDirection multiple)
{
	assert(multiple == TimeDirection::REVERSE || multiple == TimeDirection::FORWARDS);
	receiver = static_cast<TimeDirection>(static_cast<int>(receiver)*static_cast<int>(multiple));
	return receiver;
}
TimeDirection operator*(TimeDirection l, TimeDirection r) {
	return l *= r;
}

int operator+(int frameNumber, TimeDirection direction) {
	return static_cast<int>(direction) + frameNumber;
}

std::ostream &operator<<(std::ostream &o, TimeDirection timeDirection)
{
	switch (timeDirection) {
		case TimeDirection::FORWARDS:
			return o << "forwards";
		case TimeDirection::INVALID:
			return o << "invalid";
		case TimeDirection::REVERSE:
			return o << "reverse";
	}
	assert(false);
	return o;
}
}
