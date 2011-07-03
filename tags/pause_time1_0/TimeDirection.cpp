#include "TimeDirection.h"
#include <cassert>
namespace hg {
void operator*=(TimeDirection& receiver, int multiple)
{
    assert(multiple == -1 || multiple == 1);
    receiver = static_cast<TimeDirection>(receiver*multiple);
}

void operator*=(TimeDirection& receiver, TimeDirection multiple)
{
    assert(multiple == REVERSE || multiple == FORWARDS);
    receiver = static_cast<TimeDirection>(receiver*multiple);
}
}
