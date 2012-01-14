#ifndef HG_GLITZ_PERSISTER_H
#define HG_GLITZ_PERSISTER_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include "ObjectAndTime.h"
#include "Glitz.h"
#include "Frame_fwd.h"
#include "ConstPtr_of_fwd.h"

namespace hg {
class GlitzPersister : boost::totally_ordered<GlitzPersister>
{
public:
    GlitzPersister(Glitz const& glitz, unsigned lifetime, TimeDirection timeDirection);
    ObjectAndTime<GlitzPersister, Frame*> runStep(Frame* frame) const;
    Glitz const& getGlitz() const;
    TimeDirection getTimeDirection() const { return timeDirection_; }
    
    bool operator==(GlitzPersister const& other) const;
    bool operator<(GlitzPersister const& second) const;
private:
    Glitz glitz_;
    unsigned framesLeft_;
    TimeDirection timeDirection_;
};

class GlitzPersisterConstPtr : boost::totally_ordered<GlitzPersisterConstPtr>
{
public:
    GlitzPersisterConstPtr(GlitzPersister const& glitzPersister) : glitzPersister_(&glitzPersister) {}
    typedef GlitzPersister base_type;
    GlitzPersister const& get() const   { return *glitzPersister_; }
    
    ObjectAndTime<GlitzPersister, Frame*> runStep(Frame* frame) const { return glitzPersister_->runStep(frame); }
    Glitz const& getGlitz() const { return glitzPersister_->getGlitz(); }
    TimeDirection getTimeDirection() const { return glitzPersister_->getTimeDirection(); }

    bool operator==(GlitzPersisterConstPtr const& other) const { return *glitzPersister_ == *other.glitzPersister_; }
    bool operator<(GlitzPersisterConstPtr const& other) const { return *glitzPersister_ < *other.glitzPersister_; }
private:
    GlitzPersister const* glitzPersister_;
};

template<>
struct ConstPtr_of<GlitzPersister> {
    typedef GlitzPersisterConstPtr type;
};

}
#endif //HG_GLITZ_PERSISTER_H
