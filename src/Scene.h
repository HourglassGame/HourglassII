#ifndef HG_SCENE_H
#define HG_SCENE_H
#include <memory>
#include <vector>
namespace hg {
    struct WindowClosed_exception{};
    struct RunALevel_tag{
        RunALevel_tag(RunALevel_tag const&) = default;
        RunALevel_tag(RunALevel_tag &&) noexcept = default;
    };
    struct RunAReplay_tag{
        RunAReplay_tag(RunAReplay_tag const&) = default;
        RunAReplay_tag(RunAReplay_tag &&) noexcept = default;
    };
    struct Exit_tag{
        Exit_tag(Exit_tag const&) = default;
        Exit_tag(Exit_tag &&) noexcept = default;
    };
    struct LoadingCanceled_tag{};
    //struct WindowClosed_tag{};
    struct ReloadLevel_tag{};
    struct GameWon_tag{};
    struct GameAborted_tag{};
    struct SceneAborted_tag{};
}

#endif //HG_SCENE_H
