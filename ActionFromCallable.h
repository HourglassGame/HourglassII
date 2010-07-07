#ifndef HG_VOIDDEFERREDFUNCTIONCALL_H
#define HG_VOIDDEFERREDFUNCTIONCALL_H
/*
 *  VoidDeferredFunctionCall.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 6/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include "Action.h"
#include "Tracer.h"
namespace hg {
    template<class Callable>
    class ActionFromCallable : public Action {
    public:
        virtual ~ActionFromCallable()
        {
            HG_TRACE_FUNCTION
        }
        ActionFromCallable(Callable function) :
        function_(function)
        {
            HG_TRACE_FUNCTION
        }
        
    private:
        Callable function_;
        void DoCall()
        {
            HG_TRACE_FUNCTION
            function_();
        }
    };
}
#endif //HG_VOIDDEFERREDFUNCTIONCALL_H
