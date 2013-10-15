#include "ReplayIO.h"
#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
namespace hg {
    void saveReplay(std::string const &filename, std::vector<InputList> const &replay)
    {
        std::ofstream ofs(filename.c_str());
        saveReplay(ofs, replay);
    }
    std::vector<InputList> loadReplay(std::string const &filename)
    {
        std::ifstream ifs(filename.c_str());
        return ifs.is_open() ? loadReplay(ifs) : std::vector<InputList>();
    }
    void saveReplay(std::ostream &outputStream, std::vector<InputList> const &replay)
    {
        boost::archive::text_oarchive oa(outputStream);
        oa << replay;
    }
    std::vector<InputList> loadReplay(std::istream &inputStream)
    {
        std::vector<InputList> replay;
        boost::archive::text_iarchive ia(inputStream);
        ia >> replay;
        return replay;
    }
}
