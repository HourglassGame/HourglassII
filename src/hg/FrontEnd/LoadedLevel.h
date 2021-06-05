#ifndef HG_LOADED_LEVEL_H
#define HG_LOADED_LEVEL_H
#include "hg/TimeEngine/TimeEngine.h"
#include "ResourceManager.h"
namespace hg{
	struct LoadedLevel final {
		TimeEngine timeEngine;
		LevelResources resources;
	};
}
#endif //HG_LOADED_LEVEL_H
