#include "InitialScene.h"
#include "TestDriver.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio.hpp>

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>
#include <iosfwd>
#include <memory>
#include <mutex>
#ifdef _WIN32
#include <Windows.h>
#endif




namespace hg {
    extern sf::Font const *defaultFont;
           sf::Font const *defaultFont;
}

namespace {
    int run_main(std::vector<std::string> const &args);
    void initialseCurrentPath(std::vector<std::string> const &args);
    void initialiseStdIO();
    struct GlobalResourceHolder {
        GlobalResourceHolder() :
            defaultFontHolder()
        {
            std::string const filename("Arial.ttf");
            bool const fontLoaded(defaultFontHolder.loadFromFile(filename));
            assert(fontLoaded);
            hg::defaultFont = &defaultFontHolder;
        }
    private:
        sf::Font defaultFontHolder;
    };
}

//Exceptions and HourglassII:
//Unless otherwise specified, all functions in HourglassII can throw std::bad_alloc.
//Unless otherwise specified, all functions in HourglassII provide the weak exception safety guarantee.
//The exact meaning of the above statement is somewhat ambiguous, so don't assume anything about objects
//which may have been modified by a function which has thrown an exception, without checking the documentation
//(please add documentation if it is missing).
//All exceptions other than std::bad_alloc should be explicitly documented, but this is not uniformly done.
int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv+argc);

    initialiseStdIO();

    initialseCurrentPath(args);
    GlobalResourceHolder global_resources;

    if (!hg::getTestDriver().passesAllTests()) {
        std::cerr << "Failed self-check! Aborting." << std::endl;
        return EXIT_FAILURE;
    }

    return run_main(args);
}
#ifdef WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
    return main(__argc, __argv);
}
#endif



#ifdef _WIN32
/// \brief This class is a derivate of basic_stringbuf which will output all the written data using the OutputDebugString function
template<typename TChar, typename TTraits = std::char_traits<TChar>>
class OutputDebugStringBuf : public std::basic_stringbuf<TChar, TTraits, std::allocator<TChar>> {
public:
    explicit OutputDebugStringBuf() : _buffer(256) {
        setg(nullptr, nullptr, nullptr);
        setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
    }

    static_assert(std::is_same<TChar, char>::value || std::is_same<TChar, wchar_t>::value, "OutputDebugStringBuf only supports char and wchar_t types");

    int sync() try {
        std::lock_guard<std::mutex> lock(_mutex);
        MessageOutputer<TChar, TTraits>()(pbase(), pptr());
        setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
        return 0;
    }
    catch (...) {
        return -1;
    }

    int_type overflow(int_type c = TTraits::eof()) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto syncRet = sync();
        if (c != TTraits::eof()) {
            _buffer[0] = c;
            setp(_buffer.data(), _buffer.data() + 1, _buffer.data() + _buffer.size());
        }
        return syncRet == -1 ? TTraits::eof() : 0;
    }


private:
    std::mutex              _mutex;
    std::vector<TChar>      _buffer;

    template<typename TChar, typename TTraits>
    struct MessageOutputer;

    template<>
    struct MessageOutputer<char, std::char_traits<char>> {
        template<typename TIterator>
        void operator()(TIterator begin, TIterator end) const {
            std::string s(begin, end);
            OutputDebugStringA(s.c_str());
        }
    };

    template<>
    struct MessageOutputer<wchar_t, std::char_traits<wchar_t>> {
        template<typename TIterator>
        void operator()(TIterator begin, TIterator end) const {
            std::wstring s(begin, end);
            OutputDebugStringW(s.c_str());
        }
    };
};
#endif

namespace  {

void initialseCurrentPath(std::vector<std::string> const &args)
{
#if defined(__APPLE__) && defined(__MACH__)
    assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"../Resources/");
#elif defined(_WIN32)
    assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"data/");
#else
#error "Configure resource path for your platform"
#endif
}


void initialiseStdIO()
{
#ifdef _WIN32
    static OutputDebugStringBuf<char> charDebugOutput;
    std::cout.rdbuf(&charDebugOutput);
    std::cerr.rdbuf(&charDebugOutput);
    std::clog.rdbuf(&charDebugOutput);

    static OutputDebugStringBuf<wchar_t> wcharDebugOutput;
    std::wcout.rdbuf(&wcharDebugOutput);
    std::wcerr.rdbuf(&wcharDebugOutput);
    std::wclog.rdbuf(&wcharDebugOutput);
#endif
    std::ios::sync_with_stdio(false);
}

int run_main(std::vector<std::string> const &args) {
    //Insert argument handling here... (also possible to load from config file)
    //Arguments controlling initial state
    //  (to save developers time navigating menus and unlocking specific amounts of progress)
    //  Run a particular scene then exit (printing its output)
    //  -run,scene_name,args...
    //  Initially run a particular scene, then continue normally
    //  +run,scene_name,args...
    //Arguments controlling global parameters
    //  (enabling debug mode, developer mode (eg, unlocking all levels), logging, forcing particular resolutions, disable saving of progress,  etc)
    //Arguments for batch runs
    //  (running self-tests, testing levels, testing replays, rendering frames, getting version info, etc, in non-interactive, commandline mode)
    //  -run
    //  -testall
    //  -test,testname  (could be scoped test name, for test suites)
    
    
    
    //Top level options:
    //  Any Global Options:
    //   -mute (no sound)
    //   -silent (no printing)
    //   -resolution
    //   -force_resolution
    //   -fullscreen
    //   -display_test_results
    //   -do_expensive_tests
    //   -unlockall
    //   -dontsave
    //  One of:
    //   --help -h
    //   --version -v
    //   -run
    //   +run
    //   -testall
    //   -test
    //   Nothing
    
    return hg::run_hourglassii();
}
}
#if 0
namespace {
struct AsyncWindow {
    
};

struct UserInterface {
    UserInterface():
        keepRunning(true)
    {}


//Used by game to control user interface:
    AsyncWindow createWindow(WindowSettings) {
        
    }
    
    InputStream getInputStream() {
        
    }

    void updateView(hg::ViewChange newView) {
        UpdateSounds(soundManager, newView.sounds);
        UpdateGraphics(graphicsManager, newView.graphics);
    }

    atomic<bool> keepRunning;


//Private
    void run() {
        while (keepRunning) {
            while (input = app.GetInput()) {
                inputList.push_back(input);
            }
            
            graphicsManager.drawTo(renderTarget);
        }
    }

    
    hg::View latestView;
    
    Window window; //Managing stuff relating to the window *other than* drawing and input.
    RenderTarget renderTarget;
    SoundManager soundManager;
    GraphicsManager graphicsManager;
    InputSource inputSource;
};


int new_run_main(std::vector<std::string> const &args) {
    hg::UserInterface ui;
    boost::thread gameThread(run_game, ui);
    ui.run();
    gameThread.join();
    return 0;
}

static void run_game(hg::UserInterface &ui) {
    InputStream inputStream(ui.getInputStream());
    WindowHandle mainWindow(ui.createWindow(sf::VideoMode(640, 480), "Hourglass II"));

    GameState state;
    while (true) {
        ViewChange newView(UpdateGameState(state, inputList));

        ui.PlaySounds(sounds);
        FillView(newView);
        Sleep();
    }
}


/*
Screen Stack
Screens communicate via signal/slot mechanism

 Direct load Command Line option
 Main Menu
  Play Level -> Level Selection Menu -> Playing Level
  Load Replay -> Level Selection Menu -> Replay Selection Menu -> Playing Replay
  Options
  Exit
 Level Selection Menu -?> Replay Selection Menu
  search function?
  List of Levels
  Load from Filesystem
  Drag&Drop Level
 Loading Game -> Playing Level/Replay
 Playing Level/Replay
  Win Level -> Post Level Screen
  Ingame menu
 Ingame Menu
  //Notes:
      Pauses game
      Gives timeline viewer
  Timeline Viewer:
    Scrub-scroll, step, and normal-speed run through absolute timeline.
    Step forward through relative timeline
    Specify input (if running from replay, requires that the replay be "unlocked" with a "padlock" button)
  Resume
  Restart
  Take control (when in replay mode)
  Load Replay
  Save Replay
  Options
  Exit
 Post Level Screen
  //Notes:
      Timeline Viewer available
  Watch Replay (absolute, relative)
  Save Replay
  Revise Replay
  Restart Level
  Continue
 Lua (and other) Error Screem
  Copy Error to Clipboard
  Reload Level (and replay inputs?)
  Save Replay
  Abort
 Critical Bug Popup
  Copy Error to Clipboard
  Crash (to trigger debugger in appropriate place)
  Abort
*/

int new_run_main(std::vector<std::string> const &args) {
    //TODO -- Error Handling
    hg::RenderWindow window;
    auto scenes = std::vector<hg::Scene>{};
    scenes.emplace_back(std::make_unique<hg::InitialScene>());
    while (!scenes.empty()) {
        hg::Scene currentScene(std::move(scenes.back()));
        scenes.pop_back();
        auto newScenes = currentScene.run(window);
        scenes.insert(
            scenes.end(),
            make_move_iterator(newScenes.begin()),
            make_move_iterator(newScenes.end()));
    }
    return EXIT_SUCCESS;
}
}
#endif
