#ifndef HG_DEFERREDFUNCTIONCALL_H
#define HG_DEFERREDFUNCTIONCALL_H
/*
 *  DeferredFunctionCall.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "DeferredCall.h"
#include "Tracer.h"
namespace hg {
    template<class Callable>
    class DeferredCallFromCallable : public DeferredCall
    {
    public:
        DeferredCallFromCallable(Callable function) :
        function_(function)
        {
            HG_TRACE_FUNCTION
        }
        virtual ~DeferredCallFromCallable()
        {
            HG_TRACE_FUNCTION
        }
        
    private:
        Callable function_;
        inline void DoCall_()
        {
            HG_TRACE_FUNCTION
            result_ = function_();
        }
    };
}

#endif //HG_DEFERREDFUNCTIONCALL_H
