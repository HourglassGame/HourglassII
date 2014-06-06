#ifndef HG_TIME_DIRECTION_H
#define HG_TIME_DIRECTION_H
#include <ostream>
namespace hg {
enum class TimeDirection : int {
    FORWARDS = 1,
    INVALID = 0,
    REVERSE = -1
};

TimeDirection &operator*=(TimeDirection &receiver, int multiple);
TimeDirection &operator*=(TimeDirection &receiver, TimeDirection multiple);
TimeDirection operator*(TimeDirection l, TimeDirection r);

int operator+(int, TimeDirection);

std::ostream &operator<<(std::ostream &o, TimeDirection timeDirection);
}
#endif //HG_TIME_DIRECTION_H
