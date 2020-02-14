#ifndef HG_OPERATION_INTERRUPTER_H
#define HG_OPERATION_INTERRUPTER_H
#include <tbb/spin_mutex.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4457)
#endif
#include "hg/mt/boost/container/stable_vector.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "move_function.h"
#include <boost/swap.hpp>

namespace hg {
class OperationInterrupter {
public:
    //Keeps the added interruption function
    class FunctionHandle {
    public:
        FunctionHandle() : interrupter(), iterator() {}
        FunctionHandle(
            OperationInterrupter &interrupter,
            mt::boost::container::stable_vector<move_function<void()>>::iterator iterator)
                : interrupter(&interrupter), iterator(iterator) {}
        
        FunctionHandle(FunctionHandle &&o) noexcept :
            interrupter(), iterator()
        {
            swap(o);
        }
        
        FunctionHandle &operator=(FunctionHandle &&o) noexcept
        {
            swap(o);
            return *this;
        }
        
        void swap(FunctionHandle &o) {
            std::swap(interrupter, o.interrupter);
            std::swap(iterator, o.iterator);
        }
        
        ~FunctionHandle() {
            //TODO: Reconsider the safety of the possibility of `lock` throwing in a destructor
            //      and consider alternative designs.
            //This is probably safe, spin_mutex can't throw(??) (it's just a loop).
            if (interrupter) {
                OperationInterrupter &i(*interrupter);
                tbb::spin_mutex::scoped_lock lock(i.mutex);
                i.interruptionFunctions.erase(iterator);
            }
        }
    private:
        OperationInterrupter *interrupter;
        mt::boost::container::stable_vector<move_function<void()>>::iterator iterator;
    };
    OperationInterrupter() :
        interrupted_(false),
        interruptionFunctions(),
        mutex()
    {
    }
    //Calls all currently active interruptionFunctions
    //Causes attempts to add new interruptionFunctions to immediately call
    //the passed interruptionFunction.
    void interrupt();
    bool interrupted() const;
    
    //While the returned FunctionHandle (or a FunctionHandle moved from the returned FunctionHandle) is alive,
    //a call to `interrupt` will run interruptionFunction (on the thread that called `interrupt`).
    //The returned FunctionHandle must not outlive the OperationInterrupter.
    //If `interrupt` is called prior to `addInterruptionFunction`, a call to `addInterruptionFunction`
    //will immediately execute `interruptionFunction` on the thread calling `addInterruptionFunction`.
    virtual FunctionHandle addInterruptionFunction(move_function<void()> interruptionFunction);

private:
    bool interrupted_;
    mt::boost::container::stable_vector<move_function<void()>> interruptionFunctions;
    //Using tbb::spin_mutex because it is able to be locked without the possibility of failing and throwing an exception.
    //This is needed, because FunctionHandle must be able to lock on the mutex within its destructor.
    //(In theory, the other mutexes could probably be reimplemented with that guarantee added, but working from
    //the specified interface, it is not safe to just use them for this).
    mutable tbb::spin_mutex mutex;
};

struct NullOperationInterrupter final : OperationInterrupter {
    virtual FunctionHandle addInterruptionFunction(move_function<void()> interruptionFunction) override;
};
inline void swap(OperationInterrupter::FunctionHandle &l, OperationInterrupter::FunctionHandle &r) { l.swap(r); }


}//namespace hg
#endif //HG_OPERATION_INTERRUPTER_H
