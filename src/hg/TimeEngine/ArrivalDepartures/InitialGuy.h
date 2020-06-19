#ifndef HG_INITIAL_GUY_H
#define HG_INITIAL_GUY_H
#include "Guy.h"
namespace hg {
struct InitialGuy final {
	explicit InitialGuy(Guy const &nguy) : guy(nguy) {}
	Guy guy;
};
}
#endif //HG_INITIAL_GUY_H
