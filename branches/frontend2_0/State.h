#ifndef HG_STATE_H
#define HG_STATE_H
#include <SFML/Window/Event.hpp>
namespace hg {
class State {
public:
//called whenever the state becomes active after another state was previously active
//Allows re-polling of settings, window setup, player progress etc without requiring such polling to occur every frame
virtual void init(){}
//called once a frame
virtual void update(){}
virtual ~State() = 0;
};
}
#endif //HG_STATE_H
