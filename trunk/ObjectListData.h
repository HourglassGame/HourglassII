#ifndef HG_OBJECT_LIST_DATA_H
#define HG_OBJECT_LIST_DATA_H
#include "Guy.h"
#include "Box.h"
#include "Button.h"
#include "Platform.h"
#include "Portal.h"
#include "RemoteDepartureEdit.cpp"
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
    ::std::vector<Platform> platformList;
    ::std::vector<Portal> portalList;
    ::std::vector<RemoteDepartureEdit<Guy> > guyThiefList;
    ::std::vector<RemoteDepartureEdit<Box> > boxThiefList;
    ::std::vector<RemoteDepartureEdit<Guy> > guyExtraList;
    ::std::vector<RemoteDepartureEdit<Box> > boxExtraList;
};
}
#endif //HG_OBJECT_LIST_DATA_H
