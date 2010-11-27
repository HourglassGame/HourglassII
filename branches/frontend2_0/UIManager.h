#ifndef HG_UI_MANAGER_H
#define HG_UI_MANAGER_H
#include "NewFrameID.h"
#include <boost/array.hpp>
namespace hg {
class UIManager {
public:
//InputTranslator converts sf::Events into logical game-engine events
//UpdateUI uses these to update the state of the user interface
//either returns the player-input for the step or throws(is this good???)
//how about if the UI decides that it doesn't want to update the game?
//How does it decide what to draw??
//return union/polymorphic object which does what has tobe done?
//why not just do it itself then?

//Returns union, the union holds the relevant information for each
// possible UI option.
//Templated with functors for all its possible choices
//UpdateUI(InputTranslator& translator);
//Timeline timeline_;
private:
boost::array<int, 2> spacialCursorPosition;
NewFrameID temporalCursorPosition;
};
}
#endif //HG_UI_MANAGER_H
