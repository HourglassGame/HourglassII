#ifndef HG_REPLAY_IO_H
#define HG_REPLAY_IO_H
#include "InputList.h"
#include <vector>
#include <string>
namespace hg {
    void saveReplay(std::string const& filename, std::vector<InputList> const& replay);
    std::vector<InputList> loadReplay(std::string const& filename);
}
#endif //HG_REPLAY_IO_H
