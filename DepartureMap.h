#ifndef HG_DEPARTURE_MAP_H
#define HG_DEPARTURE_MAP_H
#include <boost/unordered_map.hpp>
#include <map>

#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"

#include "ObjectList.h"
#include "ObjectListTypes.h"

namespace hg {
//MASSIVE CODE DUPLICATION. Fix this at some time.

class DepartureMap {
    typedef
    boost::unordered_map<Frame*, std::map<Frame*, ObjectList<Normal> > > MapType;
public:
    typedef MapType::value_type value_type;
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;
    DepartureMap();
    //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
    void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
    void setDeparture(Frame* time, std::map<Frame*, ObjectList<Normal> >& departingObjects);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
private:
    MapType map_;
};

class EditDepartureMap {
    typedef
    boost::unordered_map<Frame*, std::map<Frame*, ObjectList<Edit> > > MapType;
public:
    typedef MapType::value_type value_type;
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;
    EditDepartureMap();
    //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
    void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
    void setDeparture(Frame* time, std::map<Frame*, ObjectList<Edit> >& departingObjects);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
private:
    MapType map_;
};

class RawDepartureMap {
    typedef
    boost::unordered_map<Frame*, std::map<Frame*, ObjectList<Normal> > > MapType;
public:
    typedef MapType::value_type value_type;
    typedef MapType::iterator iterator;
    typedef MapType::const_iterator const_iterator;
    RawDepartureMap();
    //MUST be called with all the times which will be passed to addDeparture before calling addDeparture
    void makeSpaceFor(const FrameUpdateSet& toMakeSpaceFor);
    void setDeparture(Frame* time, std::map<Frame*, ObjectList<Normal> >& departingObjects);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
private:
    MapType map_;
};
}

#endif //HG_DEPARTURE_MAP_H
