#ifndef HG_PLAYER_INPUT_H
#define HG_PLAYER_INPUT_H
#include "InputList.h"
#include <vector>
namespace hg {
class PlayerInput {
    explicit PlayerInput(std::vector<InputList> const& input) :
        input_(&input),
        isNextPlayerFrame_(false)
    {
    }
    boost::optional<InputList> getInput(std::size_t index)
    {
        mt::std::vector<InputList> const& input(input_);
        if (index < input.size()) {
            return boost::optional<InputList>(input[index]);
        } else {
            isNextPlayerFrame_ = true;
            return boost::optional<InputList>();
        }
    }
private:
    std::vector<InputList> const* input_;
    bool isNextPlayerFrame_;
};
}
#endif
