#ifndef HG_GLITZ_IMPLEMENTATION_H
#define HG_GLITZ_IMPLEMENTATION_H
#include "unique_ptr.h"
namespace hg {
class GlitzImplementation {
public:
    virtual void display(Canvas& canvas) const = 0;
    virtual std::size_t clone_size() const = 0;
    virtual GlitzImplementation* perform_clone(void* memory) const = 0;

    virtual ~GlitzImplementation(){}
private:
    virtual int order_ranking() const = 0;
    virtual bool operator<(GlitzImplementation const& other) const = 0;
    virtual bool operator==(GlitzImplementation const& other) const = 0;
    friend class Glitz;
};
}


#endif //HG_GLITZ_IMPLEMENTATION_H
