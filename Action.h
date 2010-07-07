#ifndef HG_VOIDDEFERREDCALL_H
#define HG_VOIDDEFERREDCALL_H
/*
 *  VoidDeferredCall.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 6/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <boost/any.hpp>
#include "Tracer.h"
namespace hg {
    class Action 
    {
    public:
        virtual ~Action()
        {
            HG_TRACE_FUNCTION
        }
        void Call(){
            HG_TRACE_FUNCTION
            DoCall();
        }
        //Hopeing for RVO
        void operator()()
        {
            HG_TRACE_FUNCTION
            Call();
        }
    private:
        virtual void DoCall() = 0;
    };
}
#endif //HG_VOIDDEFERREDCALL_H
