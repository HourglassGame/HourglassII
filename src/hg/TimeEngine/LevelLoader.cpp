#include "LevelLoader.h"
#include <vector>

#include "hg/LuaUtil/SimpleLuaCpp.h"
#include "lua/lualib.h"
#include "hg/LuaUtil/LuaUtilities.h"
#include "hg/PhysicsEngine/TriggerSystem/SimpleConfiguredTriggerSystem.h"
#include "hg/Util/file_util.h"
#include "hg/GlobalConst.h"

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
				loadFileIntoVector(translateToActualPath(packageName, levelPath, path{"static/SimpleConfiguredProxyLuaGlobals"})));
		}));

	lua_getfield(L, -1, "speedOfTimeTriggerID");
	bool hasSpeedOfTimeTrigger = !lua_isnil(L, -1);
	int speedOfTimeTriggerID = hasSpeedOfTimeTrigger ? lua_index_to_C_index(to<int>(L)) : -1;
	lua_pop(L, 1);

	lua_getfield(L, -1, "paradoxPressureTriggerID");
	bool hasParadoxPressureTrigger = !lua_isnil(L, -1);
	int paradoxPressureTriggerID = hasParadoxPressureTrigger ? lua_index_to_C_index(to<int>(L)) : -1;
	lua_pop(L, 1);

	return TriggerSystem{std::make_unique<SimpleConfiguredTriggerSystem>(
		std::vector<char>(system.begin(), system.end()),
		luaFiles,
		std::move(readField<TriggerOffsetsAndDefaults>(L, "triggerOffsetsAndDefaults").value),
		hasSpeedOfTimeTrigger,
		speedOfTimeTriggerID,
		hasParadoxPressureTrigger,
		paradoxPressureTriggerID,
		readField<int>(L, "arrivalLocationsSize")
	)};
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

	lua_getfield(L, -1, "speedOfTimeTriggerID");
	bool hasSpeedOfTimeTrigger = !lua_isnil(L, -1);
	int speedOfTimeTriggerID = hasSpeedOfTimeTrigger ? lua_index_to_C_index(to<int>(L)) : -1;
	lua_pop(L, 1);

	lua_getfield(L, -1, "paradoxPressureTriggerID");
	bool hasParadoxPressureTrigger = !lua_isnil(L, -1);
	int paradoxPressureTriggerID = hasParadoxPressureTrigger ? lua_index_to_C_index(to<int>(L)) : -1;
	lua_pop(L, 1);

	return TriggerSystem{std::make_unique<DirectLuaTriggerSystem>(
		std::vector<char>(system.begin(), system.end()),
		luaFiles,
		std::move(readField<TriggerOffsetsAndDefaults>(L, "triggerOffsetsAndDefaults").value),
		hasSpeedOfTimeTrigger,
		speedOfTimeTriggerID,
		hasParadoxPressureTrigger,
		paradoxPressureTriggerID,
		readField<int>(L, "arrivalLocationsSize")
	)};
}

static TriggerSystem loadTriggerSystem(lua_State *L, char const *fieldName, path const &levelPath) {
	lua_getglobal(L, fieldName);
	std::string type(readField<std::string>(L, "type"));
	if (type == "DirectLua") {
		return loadDirectLuaTriggerSystem(L, levelPath);
	}
	else if (type == "ConfiguredLua") {
		return loadSimpleConfiguredTriggerSystem(L, levelPath);
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

	unsigned speedOfTime(hg::SPEED_OF_TIME_OVERRIDE > -1 ? hg::SPEED_OF_TIME_OVERRIDE : readGlobal<int>(L, "speedOfTime"));
	unsigned speedOfTimeFuture(readGlobalWithDefault<int>(L, "speedOfTimeFuture", hg::SPEED_OF_TIME_FUTURE_DEFAULT > -1 ? hg::SPEED_OF_TIME_FUTURE_DEFAULT : speedOfTime));
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
		std::move(speedOfTimeFuture),
		std::move(timelineLength),
		std::move(environment),
		std::move(initialArrivals),
		std::move(guyArrival),
		std::move(guyStartTime),
		std::move(triggerSystem)};
}
} //namespace hg
