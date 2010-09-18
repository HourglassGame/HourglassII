#ifndef HG_OBJECT_LIST_DATA_H
#define HG_OBJECT_LIST_DATA_H
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include <vector>
namespace hg {
struct ObjectListData {
    ObjectListData();

    ObjectListData(const ObjectListData& other);
    void add(const ObjectListData& other);

    void sortElements();
    ::std::vector<Guy> guyList;
    ::std::vector<Box> boxList;
    ::std::vector<Button> buttonList;
};
}
#endif //HG_OBJECT_LIST_DATA_H