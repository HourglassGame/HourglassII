/*
 *  ExclusiveReference.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <boost/thread/mutex.hpp>
//TODO later for potential speedup in enginethreadmediator. would need to be given auto_ptr like wrapper
//also, const and non-const version
template<class T, class Lockable = boost::mutex>
class ExclusiveReverence
{
public:
    ExclusiveReference(T& object, Lockable& lockedMutex)
    {
        
    }
    
    ~ExclusiveReference()
    {
        
    }
}