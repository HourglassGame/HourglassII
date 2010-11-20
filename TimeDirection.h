#ifndef HG_TIME_DIRECTION_H
#define HG_TIME_DIRECTION_H
namespace hg {
    enum TimeDirection {
        FORWARDS = 1,
        INVALID = 0,
        REVERSE = -1
    };
    void operator*=(TimeDirection& receiver, int multiple);
    void operator*=(TimeDirection& receiver, TimeDirection multiple);
}
#endif //HG_TIME_DIRECTION_H
