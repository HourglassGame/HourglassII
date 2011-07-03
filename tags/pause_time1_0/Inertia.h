#ifndef HG_INERTIA_H
#define HG_INERTIA_H
#include "FrameID.h"
#include "TimeDirection.h"
namespace hg {
class Inertia {
    public:
    void save(const FrameID& frame, TimeDirection direction)
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
