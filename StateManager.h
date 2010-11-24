#ifndef HG_STATE_MANAGER_H
#define HG_STATE_MANAGER_H
#include <vector>
namespace hg {
class State;
class StateManager {
public:
StateManager();
//Changes the state at the top of the stack to instead be this one.
//The old current state must not access anything through its this pointer after the call to this function
//precondition: !this->empty()
//state must have been dynamically allocated with new, this transfers ownership to the StateManager
void set(State* state);
//pushes state to the top of the state-stack
//state must have been dynamically allocated with new, this transfers ownership to the StateManager
void push(State* state);
//pops the top of the state-stack
//The old current state must not access anything through its this pointer after the call to this function
void pop();
void popAll();
//returns the current state. Any previously current states are deleted during this call.
//precondition: !this->empty()
State& getCurrentState();
//Has no current state to return.
bool empty() const;
//Deletes any states still alive
//precondtion: this->empty()
~StateManager();
private:

::std::vector<State*> stateStack;
bool currentStateInitialised;
//intentionally undefined
StateManager(const StateManager& other);
StateManager& operator=(const StateManager& other);    
};
}
#endif //HG_STATE_MANAGER_H
