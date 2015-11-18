#include "LuaUtilities.h"
#include "TestDriver.h"
namespace hg {
namespace LuaUtilities_test {
namespace {
    //Check that CRLF endings do not lead to any problems for loadFileIntoVector.
    //The test file contains 10 "\r\n" newlines; we want the resulting vector to either
    //contain exactly that, or to contain 10 "\n" newlines.
    bool test_loadFileIntoVectorCRLF()
    {
        std::size_t num_newlines(10);
        auto const fileVec{ loadFileIntoVector("test/loadFileIntoVectorCRLFTest.bin") };
        auto const makeRepeatedVec = [](auto const& toRepeat, std::size_t const times)
        {
            std::vector<char> out;
            for (std::size_t i(0); i != times; ++i)
            {
                boost::push_back(out, toRepeat);
            }
            return out;
        };
        std::vector<char> const lfVec  { '\n' };
        std::vector<char> const crlfVec{ '\r', '\n' };
        return fileVec == makeRepeatedVec(lfVec, num_newlines)
            || fileVec == makeRepeatedVec(crlfVec, num_newlines);
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest("LuaUtilities_loadFileIntoVector", test_loadFileIntoVectorCRLF);
        }
    } tester;
}
}
}
