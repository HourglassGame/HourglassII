#ifndef HG_FOREACH_H
#define HG_FOREACH_H
#include <boost/foreach.hpp>

#ifdef ECLIPSE_PARSER
    //Be aware that this could be problematic if a or b are themselves macros
    #define foreach(a, b) for(a : b)
#else
    #define foreach BOOST_FOREACH
#endif

#endif HG_FOREACH_H
