#ifndef HG_DEFERREDCALLWRAPPER
#define HG_DEFERREDCALLWRAPPER
/*
 *  DeferredCallWrapper.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 7/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "Tracer.h"
#include "Action.h"
#include <boost/noncopyable.hpp>

namespace hg {
    //Saves the DeferredCall* from being deleted
    class DeferredCallWrapper : public Action, private boost::noncopyable {
    public:
        DeferredCallWrapper(DeferredCall* wrapped):
        wrapped_(wrapped)
        {
            HG_TRACE_FUNCTION
        }
        //We specificallt DON'T delete wrapped_
        virtual ~DeferredCallWrapper()
        {
            HG_TRACE_FUNCTION
        }
        inline void DoCall()
        {
            HG_TRACE_FUNCTION
            wrapped_->DoCall();
        }
    private:
        DeferredCall* wrapped_;
    };
}

#endif //HG_DEFERREDCALLWRAPPER
