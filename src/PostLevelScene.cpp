#include "PostLevelScene.h"
#include "RenderWindow.h"
#include "TimeEngine.h"

#include "Inertia.h"
#include "Scene.h"
#include "GlobalConst.h"
#include "GameDisplayHelpers.h"
#include "Maths.h"
#include "ReplayIO.h"
#include <SFML/Graphics/Text.hpp>
#include "LoadedLevel.h"

#include <sstream>

#include "PostLevelSceneRenderer.h"

namespace hg {
namespace {
    void drawFrame(
        sf::RenderTarget& target,
        VulkanEngine& eng,
        LevelResources const& resources,
        sf::Image const& wallImage,
        sf::Image const& positionColoursImage,
        std::vector<std::vector<GuyFrameData>> const& guyFrameData,
        Wall const& wall,
        std::size_t const timelineLength,
        std::vector<GuyInput> const& postOverwriteInput,
        PostLevelSceneUiFrameState const& uiFrameState);

    PostLevelSceneUiFrameState runStep(
        TimeEngine const& timeEngine,
        RenderWindow& app,
        GLFWWindow& windowglfw,
        Inertia& inertia);
}
void run_post_level_scene(
    hg::RenderWindow &window,
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
        eng.logicalDevice.device,
        eng.surface.surface,
        eng.renderPass.renderPass,
        eng.swapChain.extent,
        eng.logicalDevice.graphicsQueue,
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


    //hg::unique_ptr<hg::OperationInterrupter> interrupter(new hg::OperationInterrupter());
    hg::TimeEngine const &timeEngine = finalLevel.timeEngine;
    hg::LevelResources const &levelResources = finalLevel.resources;
    sf::Image const &wallImage = *finalLevel.bakedWall;
    sf::Image const &positionColoursImage = *finalLevel.bakedPositionColours;

    //enum {AWAITING_INPUT, RUNNING_LEVEL, PAUSED} state(AWAITING_INPUT);

    //hg::Input input;
    //input.setTimelineLength(timeEngine.getTimelineLength());
    hg::Inertia inertia;
    inertia.save(FrameID(0, UniverseID(timeEngine.getTimelineLength())), TimeDirection::FORWARDS);
    
    auto audioPlayingState = AudioPlayingState(finalLevel.resources.sounds);
    auto audioGlitzManager = AudioGlitzManager();
    
    //std::vector<hg::InputList> replay;
    //std::vector<hg::InputList>::const_iterator currentReplayIt(replay.begin());
    //std::vector<hg::InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    //std::ofstream replayLogOut("replayLogOut");
    //boost::future<hg::TimeEngine::RunResult> futureRunResult;
    //bool runningFromReplay(false);
    while (window.isOpen()) {
        glfwPollEvents();
        {
            if (glfwWindowShouldClose(windowglfw.w)) {
                window.close();
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

            auto uiFrameState{runStep(timeEngine, window, windowglfw, inertia)};

            PlayAudioGlitz(
                uiFrameState.drawnGlitz,
                audioPlayingState,
                audioGlitzManager,
                -1//guyIndex
            );

            drawFrame(
                window.getRenderTarget(),
                eng,
                levelResources,
                wallImage,
                positionColoursImage,
                guyFrameData,
                timeEngine.getWall(),
                gsl::narrow_cast<std::size_t>(timeEngine.getTimelineLength()),
                timeEngine.getPostOverwriteInput(),
                uiFrameState);

            renderer.setUiFrameState(std::move(uiFrameState));

            //Fast-Forward
            if (glfwGetKey(windowglfw.w, GLFW_KEY_F) == GLFW_PRESS) {
                window.setFramerateLimit(0);
                window.setVerticalSyncEnabled(false);
            }
            else {
                window.setFramerateLimit(hg::FRAMERATE);
                window.setVerticalSyncEnabled(true);
            }
            window.display();
            //break;
        }
        //continuemainloop:;
    }
    //breakmainloop:;
    //timeEngineThread.interrupt();
    //timeEngineThread.join();

    //return EXIT_SUCCESS;
    return;// WindowClosed_tag{};


}
namespace {
void drawFrame(
    sf::RenderTarget &target,
    hg::VulkanEngine& eng,
    hg::LevelResources const& resources,
    sf::Image const& wallImage,
    sf::Image const& positionColoursImage,
    std::vector<std::vector<GuyFrameData>> const &guyFrameData,
    Wall const &wall,
    std::size_t const timelineLength,
    std::vector<GuyInput> const &postOverwriteInput,
    PostLevelSceneUiFrameState const &uiFrameState)
{
    target.clear(sf::Color(255, 255, 255));

    DrawVisualGlitzAndWall(
        target,
        eng,
        uiFrameState.drawnGlitz,
        wall,
        resources,
        wallImage,
        positionColoursImage,
        -1,//guyIndex
        uiFrameState.shouldDrawGuyPositionColours);

    DrawTimeline2(
        target,
        timelineLength,
        TimeEngine::FrameListList{},
        uiFrameState.drawnFrame,
        postOverwriteInput.back().getTimeCursor(),
        guyFrameData,
        wall
    );
    DrawPersonalTimeline2(
        target,
        wall,
        0/*relativeGuyIndex*/,
        guyFrameData,
        postOverwriteInput,
        timelineLength
    );

    DrawInterfaceBorder(target);

    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << (std::size(guyFrameData) - 1);
        sf::Text currentPlayerGlyph;
        currentPlayerGlyph.setFont(*hg::defaultFont);
        currentPlayerGlyph.setString(currentPlayerIndex.str());
        currentPlayerGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y) - 55);
        currentPlayerGlyph.setCharacterSize(10);
        currentPlayerGlyph.setFillColor(uiTextColor);
        currentPlayerGlyph.setOutlineColor(uiTextColor);
        target.draw(currentPlayerGlyph);
    }
    {
        std::stringstream frameNumberString;
        frameNumberString << "Frame: " << getFrameNumber(uiFrameState.drawnFrame);
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(frameNumberString.str());
        frameNumberGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) + 60);
        frameNumberGlyph.setCharacterSize(16);
        frameNumberGlyph.setFillColor(uiTextColor);
        frameNumberGlyph.setOutlineColor(uiTextColor);
        target.draw(frameNumberGlyph);
    }
    {
        std::stringstream timeString;
        timeString << "Time: " << (getFrameNumber(uiFrameState.drawnFrame) * 10 / hg::FRAMERATE) / 10. << "s";
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(timeString.str());
        frameNumberGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) + 20);
        frameNumberGlyph.setCharacterSize(16);
        frameNumberGlyph.setFillColor(uiTextColor);
        frameNumberGlyph.setOutlineColor(uiTextColor);
        target.draw(frameNumberGlyph);
    }
    /*{
        std::stringstream numberOfFramesExecutedString;
        if (!boost::empty(framesExecutedList)) {
            numberOfFramesExecutedString << *boost::begin(framesExecutedList);
            for (
                int num:
                framesExecutedList
                | boost::adaptors::sliced(1, boost::size(framesExecutedList)))
            {
                numberOfFramesExecutedString << ":" << num;
            }
        }
        sf::Text numberOfFramesExecutedGlyph;
        numberOfFramesExecutedGlyph.setFont(*hg::defaultFont);
        numberOfFramesExecutedGlyph.setString(numberOfFramesExecutedString.str());
        numberOfFramesExecutedGlyph.setPosition(580, 455);
        numberOfFramesExecutedGlyph.setCharacterSize(8.f);
        numberOfFramesExecutedGlyph.setColor(uiTextColor);
        app.draw(numberOfFramesExecutedGlyph);
    }*/
    /*{
        std::stringstream fpsstring;
        fpsstring << (1./app.GetFrameTime());
        sf::Text fpsglyph;
        fpsglyph.setFont(*hg::defaultFont);
        fpsglyph.setString(fpsstring.str());
        fpsglyph.setPosition(600, 465);
        fpsglyph.setCharacterSize(8.f);
        fpsglyph.setColor(uiTextColor);
        app.draw(fpsglyph);
    }*/
    {
        sf::Text replayGlyph;
        replayGlyph.setFont(*hg::defaultFont);
        replayGlyph.setString("You Won -- Replay");
        replayGlyph.setFillColor(sf::Color(255, 0, 0));
        replayGlyph.setOutlineColor(sf::Color(255, 0, 0));
        replayGlyph.setPosition(480, 32);
        replayGlyph.setCharacterSize(16);
        target.draw(replayGlyph);
    }
    {
        sf::Text replayGlyph;
        replayGlyph.setFont(*hg::defaultFont);
        replayGlyph.setString(", . / keys control the displayed time");
        replayGlyph.setFillColor(sf::Color(255, 0, 0));
        replayGlyph.setOutlineColor(sf::Color(255, 0, 0));
        replayGlyph.setPosition(380, 64);
        replayGlyph.setCharacterSize(16);
        target.draw(replayGlyph);
    }
}

PostLevelSceneUiFrameState runStep(
    hg::TimeEngine const &timeEngine,
    hg::RenderWindow &app,
    GLFWWindow &windowglfw,
    hg::Inertia &inertia)
{
    auto const &[drawnFrame, glitz] =
        [&]() -> std::tuple<hg::FrameID, mt::std::vector<hg::Glitz> const&> {
            if (app.getInputState().isKeyPressed(sf::Keyboard::LControl)) {
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
        app.getInputState().isKeyPressed(sf::Keyboard::LShift)/*shouldDrawGuyPositionColours*/,
        glitz
    };
}
}
}

