#include "ReplayIO.h"
#include <iostream>
namespace hg {
    void saveReplay(std::string const& filename, std::vector<InputList> const& replay);
    std::vector<InputList> loadReplay(std::string const& filename);
}
