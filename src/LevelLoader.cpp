#include "LevelLoader.h"
#include <fstream>
#include <vector>

#include "SimpleLuaCpp.h"
#include "lua/lualib.h"
#include "LuaUtilities.h"
#include "SimpleConfiguredTriggerSystem.h"
#include "ComparisonTestTriggerSystem.h"

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>

namespace hg {
using boost::filesystem::path;
namespace fs = boost::filesystem;

namespace {
    path translateToActualPath(std::string const &packageName, path const &levelPath, path const &globalModulesPath) {
        //Sanitise and translate (throw if given filename is broken or unsanitary).
        //THIS FUNCTION IS CURRENTLY RETARDED -- it allows levels to read any file on the filesystem.
        
        //For the silly first pass, packageName must either be <filename without '.lua'>
        //or 'global.'<filename without '.lua'> (later we should add a proper package system?)
        std::string const global("global.");
        if (packageName.size() >= global.size()
         && std::equal(global.begin(), global.end(), packageName.begin()))
        {
            return globalModulesPath/path(packageName.substr(global.size()) + ".lua");
        }
        
        return levelPath/path(packageName + ".lua");
    }
    #if 0
    struct LoadNamedModule {
        typedef LuaModule result_type;
        LoadNamedModule(path const &levelPath) : levelPath(&levelPath) {}
        
        LuaModule operator()(std::string const &packageName) const {
            return LuaModule(
                packageName,
                loadFileIntoVector(translateToActualPath(packageName, *levelPath)));
        }
        path const *levelPath;
    };
    #endif
}
static TriggerSystem loadSimpleConfiguredTriggerSystem(lua_State *L, path const &levelPath) {
    std::string system(readField<std::string>(L, "system"));
    std::vector<std::string> luaPackageNames(readField<std::vector<std::string> >(L, "luaFiles", -1));

    std::vector<path> luaFilePaths;
    luaFilePaths.reserve(luaPackageNames.size());

    std::vector<LuaModule> luaFiles;
    boost::push_back(luaFiles, luaPackageNames | boost::adaptors::transformed(
        [&levelPath](std::string const &packageName) {
            return LuaModule(
                packageName,
                loadFileIntoVector(translateToActualPath(packageName, levelPath, path{"SimpleConfiguredProxyLuaGlobals"})));
        }));

    return hg::make_unique<SimpleConfiguredTriggerSystem>(
          std::vector<char>(system.begin(), system.end()),
          luaFiles,
          std::move(readField<TriggerOffsetsAndDefaults>(L, "triggerOffsetsAndDefaults").value),
          readField<int>(L, "arrivalLocationsSize"));
}

static TriggerSystem loadDirectLuaTriggerSystem(lua_State *L, path const &levelPath) {
    std::string system(readField<std::string>(L, "system"));
    std::vector<std::string> luaPackageNames(readField<std::vector<std::string> >(L, "luaFiles", -1));

    std::vector<path> luaFilePaths;
    luaFilePaths.reserve(luaPackageNames.size());

    std::vector<LuaModule> luaFiles;
    boost::push_back(luaFiles, luaPackageNames | boost::adaptors::transformed(
        [&levelPath](std::string const &packageName) {
            return LuaModule(
                packageName,
                loadFileIntoVector(translateToActualPath(packageName, levelPath, path{})));
        }));

    return
      hg::make_unique<DirectLuaTriggerSystem>(
          std::vector<char>(system.begin(), system.end()),
          luaFiles,
          std::move(readField<TriggerOffsetsAndDefaults>(L, "triggerOffsetsAndDefaults").value),
          readField<int>(L, "arrivalLocationsSize"));
}

static TriggerSystem loadTriggerSystem(lua_State *L, char const *fieldName, path const &levelPath) {
    lua_getglobal(L, fieldName);
    std::string type(readField<std::string>(L, "type"));
    if (type == "DirectLua") {
        return loadDirectLuaTriggerSystem(L, levelPath);
    }
    else if (type == "CompatibleLua") {
        #ifndef NDEBUG
        return hg::make_unique<ComparisonTestTriggerSystem>(
            loadDirectLuaTriggerSystem(L, levelPath),
            loadSimpleConfiguredTriggerSystem(L, levelPath)
            );
        #else
        return loadSimpleConfiguredTriggerSystem(L, levelPath);
        #endif
    }
    else {
        std::stringstream ss;
        ss << "unrecognised triggerSystem type: " << type;
        BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info(ss.str()));
    }
}

Level loadLevelFromFile(
    boost::filesystem::path const &levelPath,
    OperationInterrupter &interrupter)
{
    std::vector<char> levelChunk(loadFileIntoVector(levelPath/path("main.lua")));
    LuaState levelGenerator((LuaState::new_state_t()));

    lua_State *L(levelGenerator.ptr);
    pushFunctionFromVector(L, levelChunk, "Level");

    LuaInterruptionHandle h(makeInterruptable(L, interrupter));

    lua_call(L, 0, 0);

    unsigned speedOfTime(readGlobal<int>(L, "speedOfTime"));
    int timelineLength(readGlobal<int>(L, "timelineLength"));
    Environment environment(readGlobal<Environment>(L, "environment"));
    ObjectList<NonGuyDynamic> initialArrivals(readGlobal<InitialObjects>(L, "initialArrivals").list);
    initialArrivals.sort();
    InitialGuyArrival initialGuy(readGlobal<InitialGuyArrival>(L, "initialGuy"));
    Guy &guyArrival(initialGuy.arrival);
    FrameID guyStartTime(initialGuy.arrivalTime, UniverseID(timelineLength));
    TriggerSystem triggerSystem(loadTriggerSystem(L, "triggerSystem", levelPath));

    return {
        std::move(speedOfTime),
        std::move(timelineLength),
        std::move(environment),
        std::move(initialArrivals),
        std::move(guyArrival),
        std::move(guyStartTime),
        std::move(triggerSystem)};
}
} //namespace hg
