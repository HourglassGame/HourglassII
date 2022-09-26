#ifndef HG_SCENE_H
#define HG_SCENE_H
#include <memory>
#include <vector>
#include "hg/TimeEngine/TimeEngine.h"
#include "hg/Util/OperationInterrupter.h"
#include "LoadedLevel.h"
namespace hg {
	struct WindowClosed_exception final {};
	struct RunALevel_tag final {
		RunALevel_tag() = default;
		RunALevel_tag(RunALevel_tag const&) = default;
		RunALevel_tag(RunALevel_tag &&) noexcept = default;
	};
	struct RunAReplay_tag final {
		RunAReplay_tag() = default;
		RunAReplay_tag(RunAReplay_tag const&) = default;
		RunAReplay_tag(RunAReplay_tag &&) noexcept = default;
	};
	struct Exit_tag final {
		Exit_tag() = default;
		Exit_tag(Exit_tag const&) = default;
		Exit_tag(Exit_tag &&) noexcept = default;
	};
	struct LoadingCanceled_tag final {};
	//struct WindowClosed_tag{};
	struct ReloadLevel_tag final {};
	struct GameWon_tag final {};
	struct GameAborted_tag final {};
	struct SceneAborted_tag final {};
	
	struct LoadLevelFunction final {
		std::string levelName;
		int position;
		int page;
		hg::move_function<hg::TimeEngine(hg::OperationInterrupter &)> timeEngineLoadFun;
		hg::move_function<hg::LoadedLevel(hg::TimeEngine &&)> glitzLoadFun;
	};
}

#endif //HG_SCENE_H
