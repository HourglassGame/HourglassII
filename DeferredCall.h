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
namespace hg {
    class DeferredCall
    {
    public:
        DeferredCall():
        result_(),
        HasResultCond(),
        ResultWaitExcl()
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
            boost::unique_lock<boost::mutex> lock(ResultWaitExcl);
            while (result_.empty()) {
                HasResultCond.wait(lock);
            }
            hg_assert(!result_.empty());
            return result_;
        }

        void DoCall()
        {
            HG_TRACE_FUNCTION
            {
                boost::lock_guard<boost::mutex> lock(ResultWaitExcl);
                hg_assert(result_.empty());
                DoCall_();
                hg_assert(!result_.empty());
            }
            HasResultCond.notify_all();
        }
    protected:
        boost::any result_;
    private:
        boost::condition_variable HasResultCond;
        boost::mutex ResultWaitExcl;

        virtual void DoCall_() = 0;
    };
}
#endif //HG_DEFERREDCALL_H
