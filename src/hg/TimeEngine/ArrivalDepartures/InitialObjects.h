#ifndef HG_INITIAL_OBJECTS_H
#define HG_INITIAL_OBJECTS_H
#include "ObjectList.h"
#include "ObjectListTypes.h"
namespace hg {
struct InitialObjects final {
	explicit InitialObjects(ObjectList<NonGuyDynamic> const &nlist) : list(nlist) {}
	ObjectList<NonGuyDynamic> list;
};
}
#endif //HG_INITIAL_OBJECTS_H
