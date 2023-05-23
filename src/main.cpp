#include "hg/FrontEnd/InitialScene.h"
#include "hg/Util/TestDriver.h"
#include "hg/Util/file_util.h"
#include "hg/Util/memory_util.h"

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

    extern std::vector<uint32_t> const *demoFragSpv;
           std::vector<uint32_t> const *demoFragSpv;

    extern std::vector<uint32_t> const *demoVertSpv;
           std::vector<uint32_t> const *demoVertSpv;
}

namespace {
    int run_main(std::vector<std::string> const &args);
    void initialiseCurrentPath(std::vector<std::string> const &args);
    void initialiseStdIO();
    struct GlobalResourceHolder {
        GlobalResourceHolder() :
            demoFragSpvHolder(),
            demoVertSpvHolder()
        {
            demoFragSpvHolder = hg::reinterpretToUint32Vector(hg::loadFileIntoVector("compiled/shaders/demo.frag.glsl.spv"));
            hg::demoFragSpv = &demoFragSpvHolder;

            demoVertSpvHolder = hg::reinterpretToUint32Vector(hg::loadFileIntoVector("compiled/shaders/demo.vert.glsl.spv"));
            hg::demoVertSpv = &demoVertSpvHolder;
        }
    private:
        std::vector<uint32_t> demoFragSpvHolder;
        std::vector<uint32_t> demoVertSpvHolder;
    };
}



//Exceptions and HourglassII:
//Unless otherwise specified, all functions in HourglassII can throw std::bad_alloc.
//Unless otherwise specified, all functions in HourglassII provide the weak exception safety guarantee.
//The exact meaning of the above statement is somewhat ambiguous, so don't assume anything about objects
//which may have been modified by a function which has thrown an exception, without checking the documentation
//(please add documentation if it is missing).
//All exceptions other than std::bad_alloc should be explicitly documented, but this is not uniformly done.
int main_entry(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);

    initialiseStdIO();

    initialiseCurrentPath(args);
    GlobalResourceHolder global_resources;

    if (!hg::getTestDriver().passesAllTests()) {
        std::cerr << "Failed self-check! Aborting." << std::endl;
        return EXIT_FAILURE;
    }
    
    return run_main(args);
}

#ifdef WIN32
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd)
{
    return main_entry(__argc, __argv);
}
#else
int main(int argc, char *argv[])
{
    return main_entry(argc, argv);
}
#endif


#ifdef _MSC_VER
/// \brief This class is a derivate of basic_stringbuf which will output all the written data using the OutputDebugString function
template<typename TChar, typename TTraits = std::char_traits<TChar>>
class OutputDebugStringBuf : public std::basic_stringbuf<TChar, TTraits, std::allocator<TChar>> {
public:

    typedef std::basic_streambuf<TChar, TTraits> StringBufT;
    explicit OutputDebugStringBuf() : _buffer(256) {
        StringBufT::setg(nullptr, nullptr, nullptr);
        StringBufT::setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
    }

    static_assert(std::is_same<TChar, char>::value || std::is_same<TChar, wchar_t>::value, "OutputDebugStringBuf only supports char and wchar_t types");

    int sync() override {
        std::lock_guard<std::mutex> lock(_mutex);
        return do_sync();
    }

    //int_type overflow(int_type c = TTraits::eof()) override
    virtual typename TTraits::int_type overflow(typename TTraits::int_type c = TTraits::eof()) override  {
        std::lock_guard<std::mutex> lock(_mutex);
        auto syncRet = do_sync();
        if (c != TTraits::eof()) {
            _buffer[0] = static_cast<TChar>(c);
            StringBufT::setp(_buffer.data(), _buffer.data() + 1, _buffer.data() + _buffer.size());
        }
        return syncRet == -1 ? TTraits::eof() : 0;
    }


private:
    int do_sync() try {
        MessageOutputer<TChar, TTraits>()(StringBufT::pbase(), StringBufT::pptr());
        StringBufT::setp(_buffer.data(), _buffer.data(), _buffer.data() + _buffer.size());
        return 0;
    }
    catch (...) {
        return -1;
    }


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

void initialiseCurrentPath(std::vector<std::string> const &args)
{
#if defined(__APPLE__) && defined(__MACH__)
    assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"../Resources/");
#else
    assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"data/");
#endif
}


void initialiseStdIO()
{
#ifdef _MSC_VER
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

int run_main(std::vector<std::string> const &) {
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
    WindowHandle mainWindow(ui.createWindow(sf::VideoMode(hg::WINDOW_DEFAULT_X, hg::WINDOW_DEFAULT_Y), "Hourglass II"));

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
