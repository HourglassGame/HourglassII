#include "TimeDirection.h"
void ::hg::operator*=(::hg::TimeDirection& receiver, int multiple)
{
    assert(multiple == -1 || multiple == 1);
    receiver = static_cast< ::hg::TimeDirection>(receiver*multiple);
}

void ::hg::operator*=(::hg::TimeDirection& receiver, ::hg::TimeDirection multiple)
{
    assert(multiple == REVERSE || multiple == FORWARDS);
    receiver = static_cast< ::hg::TimeDirection>(receiver*multiple);
}
