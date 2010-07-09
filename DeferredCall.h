#ifndef HG_DEFERREDCALL_H
#define HG_DEFERREDCALL_H
/*
 *  DeferredCall.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include "HourglassAssert.h"
#include <boost/any.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "Tracer.h"
#include <iostream>
namespace hg {
    class DeferredCall
    {
    public:
        DeferredCall():
        result_(),
        hasResultCond(),
        resultWaitExcl()
        {
            HG_TRACE_FUNCTION
        }
        
        virtual ~DeferredCall()
        {
            HG_TRACE_FUNCTION
        }
        
        boost::any GetResult()
        {
            HG_TRACE_FUNCTION
            boost::unique_lock<boost::mutex> lock(resultWaitExcl);
            while (result_.empty()) {
                hasResultCond.wait(lock);
            }
            hg_assert(!result_.empty());
            return result_;
        }

        void DoCall()
        {
            HG_TRACE_FUNCTION
            {
                boost::lock_guard<boost::mutex> lock(resultWaitExcl);
                hg_assert(result_.empty());
                DoCall_();
                hg_assert(!result_.empty());
            }
            hasResultCond.notify_all();
            return;
        }
    protected:
        boost::any result_;
    private:
        boost::condition_variable hasResultCond;
        boost::mutex resultWaitExcl;

        virtual void DoCall_() = 0;
    };
}
#endif //HG_DEFERREDCALL_H
