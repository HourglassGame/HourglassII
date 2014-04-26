#ifndef HG_GLITZ_PERSISTER_H
#define HG_GLITZ_PERSISTER_H
#include "TimeDirection.h"
#include <boost/operators.hpp>
#include <tuple>
#include "ObjectAndTime.h"
#include "Glitz.h"
#include "Frame_fwd.h"
#include "ConstPtr_of_fwd.h"

namespace hg {
class GlitzPersister : boost::totally_ordered<GlitzPersister>
{
public:
    GlitzPersister(
        Glitz const &forwardsGlitz, Glitz const &reverseGlitz,
        unsigned lifetime, TimeDirection timeDirection);
    ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const;
    Glitz const &getForwardsGlitz() const { return forwardsGlitz; }
    Glitz const &getReverseGlitz()  const { return reverseGlitz; }
    
    bool operator==(GlitzPersister const &o) const;
    bool operator<(GlitzPersister const &o) const;
private:
    Glitz forwardsGlitz;
    Glitz reverseGlitz;
    unsigned framesLeft;
    TimeDirection timeDirection;
    auto as_tie() const ->
        decltype(std::tie(
                    forwardsGlitz, reverseGlitz,
                    framesLeft,timeDirection))
    {
        return std::tie(
            forwardsGlitz, reverseGlitz,
            framesLeft,timeDirection);
    }
};

class GlitzPersisterConstPtr : boost::totally_ordered<GlitzPersisterConstPtr>
{
public:
    GlitzPersisterConstPtr(GlitzPersister const &glitzPersister) : glitzPersister(&glitzPersister) {}
    typedef GlitzPersister base_type;
    GlitzPersister const &get() const   { return *glitzPersister; }
    
    ObjectAndTime<GlitzPersister, Frame *> runStep(Frame *frame) const { return glitzPersister->runStep(frame); }
    Glitz const &getForwardsGlitz() const { return glitzPersister->getForwardsGlitz(); }
    Glitz const &getReverseGlitz() const { return glitzPersister->getReverseGlitz(); }
    
    bool operator==(GlitzPersisterConstPtr const &o) const { return *glitzPersister == *o.glitzPersister; }
    bool operator<(GlitzPersisterConstPtr const &o) const { return *glitzPersister < *o.glitzPersister; }
private:
    GlitzPersister const *glitzPersister;
};

template<>
struct ConstPtr_of<GlitzPersister> {
    typedef GlitzPersisterConstPtr type;
};

}
#endif //HG_GLITZ_PERSISTER_H
