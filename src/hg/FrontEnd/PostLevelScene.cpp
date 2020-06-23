#include "PostLevelScene.h"
#include "hg/TimeEngine/TimeEngine.h"

#include "Inertia.h"
#include "Scene.h"
#include "hg/GlobalConst.h"
#include "GameDisplayHelpers.h"
#include "hg/Util/Maths.h"
#include "ReplayIO.h"
#include "LoadedLevel.h"

#include "PostLevelSceneRenderer.h"

#include <sstream>
#include <tuple>

namespace hg {
namespace {
    PostLevelSceneUiFrameState runStep(
        TimeEngine const& timeEngine,
        GLFWWindow& windowglfw,
        Inertia& inertia);
}
void run_post_level_scene(
    GLFWWindow &windowglfw,
    VulkanEngine &eng,
    VulkanRenderer &vkRenderer,
    TimeEngine const &initialTimeEngine,
    LoadedLevel const &finalLevel)
{
    std::vector<std::vector<GuyFrameData>> guyFrameData;
    {
        auto const &guyFrames{finalLevel.timeEngine.getGuyFrames()};
        auto const actualGuyFrames{boost::make_iterator_range(guyFrames.begin(), std::prev(guyFrames.end()))};
        guyFrameData.reserve(actualGuyFrames.size());
        for (std::size_t i{}, end{ std::size(actualGuyFrames) }; i != end; ++i) {
            guyFrameData.push_back({});
            for (hg::Frame* frame : actualGuyFrames[i]) {
                guyFrameData[i].push_back(
                    GuyFrameData{
                        getFrameNumber(frame),
                        findCurrentGuy(frame->getView().getGuyInformation(), i)
                    }
                );
            }
        }
    }
    PostLevelSceneRenderer renderer(
        eng.physicalDevice,
        eng.device.h(),
        eng.surface.surface,
        eng.renderPass.h(),
        eng.swapChain.extent(),
        eng.device.graphicsQ(),
        guyFrameData,
        finalLevel.timeEngine.getWall(),
        gsl::narrow_cast<std::size_t>(finalLevel.timeEngine.getTimelineLength()),
        finalLevel.timeEngine.getPostOverwriteInput()
    );
    vkRenderer.StartScene(renderer);
    struct RendererCleanupEnforcer final {
        decltype(vkRenderer)& vkRenderer_;
        ~RendererCleanupEnforcer() noexcept {
            vkRenderer_.EndScene();
        }
    } RendererCleanupEnforcer_obj{ vkRenderer };


    hg::TimeEngine const &timeEngine = finalLevel.timeEngine;
    hg::LevelResources const &levelResources = finalLevel.resources;

    hg::Inertia inertia;
    inertia.save(FrameID(0, UniverseID(timeEngine.getTimelineLength())), TimeDirection::FORWARDS);
    
    auto audioPlayingState = AudioPlayingState(finalLevel.resources.sounds);
    auto audioGlitzManager = AudioGlitzManager();
    auto frameStartTime{std::chrono::steady_clock::now()};

    while (true) {
        glfwPollEvents();
        {
            if (glfwWindowShouldClose(windowglfw.w)) {
                //window.close();
                throw WindowClosed_exception{};
            }

            if (windowglfw.hasLastKey()) {
                int key = windowglfw.useLastKey();
                //Leave level
                if (key == GLFW_KEY_ESCAPE) {
                    return;
                }
                //Save replay
                if (key == GLFW_KEY_L) {
                    saveReplay("replay", finalLevel.timeEngine.getReplayData());
                }
                //Restart Replay

                //Pause Relative Replay

                //Resume Relative Replay
            }

            //Inertia Forwards/Backwards
            if (glfwGetKey(windowglfw.w, GLFW_KEY_PERIOD) == GLFW_PRESS) {
                inertia.save(mousePosToFrameID(windowglfw, timeEngine), TimeDirection::FORWARDS);
            }
            if (glfwGetKey(windowglfw.w, GLFW_KEY_COMMA) == GLFW_PRESS) {
                inertia.save(mousePosToFrameID(windowglfw, timeEngine), TimeDirection::REVERSE);
            }
            if (glfwGetKey(windowglfw.w, GLFW_KEY_SLASH) == GLFW_PRESS) {
                inertia.reset();
            }

            auto uiFrameState{runStep(timeEngine, windowglfw, inertia)};

            PlayAudioGlitz(
                uiFrameState.drawnGlitz,
                audioPlayingState,
                audioGlitzManager,
                -1//guyIndex
            );
            renderer.setUiFrameState(std::move(uiFrameState));

            //Fast-Forward
            if (glfwGetKey(windowglfw.w, GLFW_KEY_F) == GLFW_PRESS) {
                frameStartTime = std::chrono::steady_clock::now();
            }
            else {
                while (true) {
                    auto const t{std::chrono::steady_clock::now()};
                    //TODO: Avoid busy-wait
                    if (t >= frameStartTime+std::chrono::duration<double>(1./FRAMERATE)) {
                        frameStartTime = t;
                        break;
                    }
                }
            }
        }
    }
}
namespace {
PostLevelSceneUiFrameState runStep(
    hg::TimeEngine const &timeEngine,
    GLFWWindow &windowglfw,
    hg::Inertia &inertia)
{
    auto const &[drawnFrame, glitz] =
        [&]() -> std::tuple<hg::FrameID, mt::std::vector<hg::Glitz> const&> {
            if (glfwGetKey(windowglfw.w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                auto const drawnFrame{mousePosToFrameID(windowglfw, timeEngine)};
                return {
                    drawnFrame,
                    getGlitzForDirection(timeEngine.getFrame(drawnFrame)->getView(), TimeDirection::FORWARDS)
                };
            }
            else {
                inertia.run();
                hg::FrameID const inertialFrame(inertia.getFrame());
                if (!isNullFrame(inertialFrame)) {
                    return {
                        inertialFrame,
                        getGlitzForDirection(timeEngine.getFrame(inertialFrame)->getView(), inertia.getTimeDirection())
                    };
                }
                else {
                    auto const drawnFrame{mousePosToFrameID(windowglfw, timeEngine)};
                    return {
                        drawnFrame,
                        getGlitzForDirection(timeEngine.getFrame(drawnFrame)->getView(), TimeDirection::FORWARDS)
                    };
                }
            }
        }();

    return {
        drawnFrame,
        glfwGetKey(windowglfw.w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS/*shouldDrawGuyPositionColours*/,
        glitz
    };
}
}
}

