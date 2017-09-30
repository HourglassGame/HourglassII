#ifndef HG_INERTIA_H
#define HG_INERTIA_H
#include "FrameID.h"
#include "TimeDirection.h"
#include <cassert>
namespace hg {
class Inertia final {
    public:
    Inertia() = default;
    void reset() {
        frame_ = FrameID();
    }
    void save(FrameID const &frame, TimeDirection direction)
    {
        frame_ = frame;
        direction_ = direction;
    }
    void run()
    {
        frame_ = frame_.nextFrame(direction_);
    }
    FrameID getFrame() const {
        return frame_;
    }
    TimeDirection getTimeDirection() const {
        assert(frame_.isValidFrame());
        return direction_;
    }
    private:
    FrameID frame_;
    TimeDirection direction_;
};
}

#endif //HG_INERTIA_H
