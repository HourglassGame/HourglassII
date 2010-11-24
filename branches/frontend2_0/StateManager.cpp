#include "StateManager.h"
#include "State.h"
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
using ::std::auto_ptr;
namespace hg {
StateManager::StateManager() :
stateStack(),
currentStateInitialised(false)
{
}
//Changes the state at the top of the stack to instead be this one.
//Safe to call from within the state as the old top state is only deleted when getCurrentState is called
void StateManager::set(State* state)
{
    assert(!empty());
    delete stateStack.back();
    stateStack.back() = state;
    currentStateInitialised = false;
}
//pushes state to the top of the state-stack
void StateManager::push(State* state)
{
    auto_ptr<State> stateContainer(state);
    stateStack.push_back(state);
    currentStateInitialised = false;
    stateContainer.release();
}
//pops the top of the state-stack
//Safe to call from within the state as the old top state is only deleted when getCurrentState is called
void StateManager::pop()
{
    assert(!empty());
    delete stateStack.back();
    stateStack.pop_back();
    currentStateInitialised = false;
}
void StateManager::popAll()
{
    //Obviously could be faster, but anyway...
    while (!empty()) {
        pop();
    }
}
//returns the current state. Any previously current states are deleted during this call.
State& StateManager::getCurrentState()
{
    assert(!empty());
    if (!currentStateInitialised) {
        stateStack.back()->init();
        currentStateInitialised = true;
    }
    return *stateStack.back();
}
//Has no current state to return.
bool StateManager::empty() const
{
    return stateStack.empty();
}
//Deletes any states still alive
StateManager::~StateManager()
{
    assert(empty());
}
}
