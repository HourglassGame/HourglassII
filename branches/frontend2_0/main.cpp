#include "GameEngine.h"
#include <iostream>
#include <exception>
using namespace ::hg;
using namespace ::std;
namespace {
    bool parseOptions(int argc, char** argv, hg::GameEngine& engine);
    bool parseOptions(int /*argc*/, char** /*argv*/, hg::GameEngine& /*engine*/)
    {
        // parses command line and config file options and gives engine appropriate settings
        // Precedence: command line > player options > config > default
        // player options is a folder which contains an encrypted file for each player, 
        //      containing that player's options and progress
        // config is a single file, which contains the currently active player and all other global configuration
        //          no player options shall have the possibility of rendering the game unplayable 
        //            (we don't want a savegame to becpme corrupted just because of a stupid setting)
        //global/config options can have the possibility of screwing up the game, 
        //but the in-game interface should not allow people to select such settings (as far as possible)
        //Of course they can still do it in text editor
        //Options options; //contains default settings

        //options.setFoo(bar);

        //engine.setOptions(options);
        return true;
    }
}

int main(int argc, char** argv)
{
    try {
        GameEngine engine;
        if (parseOptions(argc, argv, engine)) {
            engine.go();
        }
    }
    catch (std::exception& e) {
        cout << "ERROR - uncaught standard exception: " << e.what() << "\n";
        cout << "Aborting" << "\n";
        return EXIT_FAILURE;
    }
    catch (...) {
        cout << "ERROR - uncaught exception: " << "unknown" << "\n";
        cout << "Aborting" << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
