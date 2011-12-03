#ifndef HG_REPLAY_IO_H
#define HG_REPLAY_IO_H
#include "InputList.h"
#include <vector>
#include <string>
#include <iosfwd>
namespace hg {
    //saveReplay and loadReplay are required to match each other, and to produce a 
    //replay that can be saved on one machine and loaded on any other
    //that is running HourglassII. That is, the replays myst be portable.
    //The current implementations approximate this, but they should be improved!
    void saveReplay(std::string const& filename, std::vector<InputList> const& replay);
    std::vector<InputList> loadReplay(std::string const& filename);

    void saveReplay(std::ostream& outputStream, std::vector<InputList> const& replay);
    std::vector<InputList> loadReplay(std::istream& inputStream);
}
#endif //HG_REPLAY_IO_H
