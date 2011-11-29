#ifndef HG_FOREACH_H
#define HG_FOREACH_H
#include <boost/foreach.hpp>

#ifdef __CDT_PARSER__
//Be aware that this could be problematic
//if a or b are themselves macros (maybe??)
#define foreach(a, b) for(a : b)
#else
#define foreach BOOST_FOREACH
#endif

#endif //HG_FOREACH_H
