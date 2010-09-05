#ifndef HG_OBJECT_LIST_DATA_H
#define HG_OBJECT_LIST_DATA_H
#include "Guy.h"
#include "Box.h"
#include <vector>
namespace hg {
struct ObjectListData {
    ObjectListData();
    
    ObjectListData(const ObjectListData& other);
    void add(const ObjectListData& other);
    
    void sortElements();
    ::std::vector<Guy> guyList;
    ::std::vector<Box> boxList;
};
}
#endif //HG_OBJECT_LIST_DATA_H
