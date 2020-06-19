#ifndef HG_INITIAL_BOX_H
#define HG_INITIAL_BOX_H
#include "Box.h"
namespace hg {
struct InitialBox final {
	explicit InitialBox(Box const &nbox) : box(nbox) {}
	Box box;
};
}
#endif //HG_INITIAL_BOX_H
