#ifndef HG_GUY_OUTPUT_INFO_H
#define HG_GUY_OUTPUT_INFO_H
#include <cstddef>
#include "TimeDirection.h"
namespace hg {
class GuyOutputInfo {
public:
    explicit GuyOutputInfo(TimeDirection timeDirection) :
        timeDirection_(timeDirection) {}
    TimeDirection getTimeDirection() const { return timeDirection_; }
private:
    TimeDirection timeDirection_;
};
}//namespace hg
#endif //HG_GUY_OUTPUT_INFO_H
