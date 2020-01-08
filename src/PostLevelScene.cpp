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

namespace hg {
namespace {
    void runStep(
        hg::TimeEngine const &timeEngine,
        hg::RenderWindow &app,
        GLFWWindow &windowglfw,
        hg::VulkanEngine &eng,
        AudioPlayingState &audioPlayingState,
        AudioGlitzManager &audioGlitzManager,
        hg::Inertia &inertia,
        hg::LevelResources const &resources,
        sf::Image const &wallImage,
        sf::Image const &positionColoursImage);
}
void run_post_level_scene(
    hg::RenderWindow &window,
    GLFWWindow &windowglfw,
    hg::VulkanEngine &eng,
    TimeEngine const &initialTimeEngine,
    LoadedLevel const &finalLevel)
{
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
            runStep(timeEngine, window, windowglfw, eng, audioPlayingState, audioGlitzManager, inertia, levelResources, wallImage, positionColoursImage);
            {
                sf::Text replayGlyph;
                replayGlyph.setFont(*hg::defaultFont);
                replayGlyph.setString("You Won -- Replay");
                replayGlyph.setFillColor(sf::Color(255,0,0));
                replayGlyph.setOutlineColor(sf::Color(255, 0, 0));
                replayGlyph.setPosition(480, 32);
                replayGlyph.setCharacterSize(16);
                window.draw(replayGlyph);
            }
            {
                sf::Text replayGlyph;
                replayGlyph.setFont(*hg::defaultFont);
                replayGlyph.setString(", . / keys control the displayed time");
                replayGlyph.setFillColor(sf::Color(255,0,0));
                replayGlyph.setOutlineColor(sf::Color(255, 0, 0));
                replayGlyph.setPosition(380, 64);
                replayGlyph.setCharacterSize(16);
                window.draw(replayGlyph);
            }
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
void runStep(
    hg::TimeEngine const &timeEngine,
    hg::RenderWindow &app,
    GLFWWindow &windowglfw,
    hg::VulkanEngine &eng,
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    hg::Inertia &inertia,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    sf::Image const &positionColoursImage)
{
    app.clear(sf::Color(255, 255, 255));
    hg::FrameID drawnFrame;

    bool const shouldDrawGuyPositionColours{ app.getInputState().isKeyPressed(sf::Keyboard::LShift) };

    if (app.getInputState().isKeyPressed(sf::Keyboard::LControl)) {
        drawnFrame = mousePosToFrameID(windowglfw, timeEngine);
        hg::Frame const *frame(timeEngine.getFrame(drawnFrame));
        auto const &glitz{ getGlitzForDirection(frame->getView(), TimeDirection::FORWARDS) };
        auto const guyIndex{-1};
        PlayAudioGlitz(
            glitz,
            audioPlayingState,
            audioGlitzManager,
            guyIndex
        );
        DrawVisualGlitzAndWall(app.getRenderTarget(),
            eng,
            glitz,
            timeEngine.getWall(),
            resources,
            wallImage,
            positionColoursImage,
            guyIndex,
            shouldDrawGuyPositionColours);
    }
    else {
        inertia.run();
        hg::FrameID const inertialFrame(inertia.getFrame());
        if (inertialFrame.isValidFrame()) {
            drawnFrame = inertialFrame;
            hg::Frame const *frame(timeEngine.getFrame(inertialFrame));
            auto const &glitz{ getGlitzForDirection(frame->getView(), inertia.getTimeDirection()) };
            auto const guyIndex{-1};
            PlayAudioGlitz(
                glitz,
                audioPlayingState,
                audioGlitzManager,
                guyIndex
                );
            DrawVisualGlitzAndWall(app.getRenderTarget(),
                eng,
                glitz,
                timeEngine.getWall(),
                resources,
                wallImage,
                positionColoursImage,
                guyIndex,
                shouldDrawGuyPositionColours);
        }
        else {
            drawnFrame = mousePosToFrameID(windowglfw, timeEngine);
            hg::Frame const *frame(timeEngine.getFrame(drawnFrame));
            auto const &glitz{ getGlitzForDirection(frame->getView(), TimeDirection::FORWARDS) };
            auto const guyIndex{-1};
            PlayAudioGlitz(
                glitz,
                audioPlayingState,
                audioGlitzManager,
                guyIndex
            );
            DrawVisualGlitzAndWall(app.getRenderTarget(),
                eng,
                glitz,
                timeEngine.getWall(),
                resources,
                wallImage,
                positionColoursImage,
                guyIndex,
                shouldDrawGuyPositionColours);
        }
    }
    DrawTimeline(app.getRenderTarget(), timeEngine, TimeEngine::FrameListList{}, drawnFrame, timeEngine.getReplayData().back().getGuyInput().getTimeCursor(), timeEngine.getTimelineLength());
    DrawPersonalTimeline(
        app.getRenderTarget(),
        timeEngine,
        0/*relativeGuyIndex*/,
        timeEngine.getGuyFrames(),
        timeEngine.getPostOverwriteInput(),
        static_cast<std::size_t>(timeEngine.getTimelineLength()));
        
    DrawInterfaceBorder(app.getRenderTarget());

    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << timeEngine.getReplayData().size() - 1;
        sf::Text currentPlayerGlyph;
        currentPlayerGlyph.setFont(*hg::defaultFont);
        currentPlayerGlyph.setString(currentPlayerIndex.str());
        currentPlayerGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y) - 55);
        currentPlayerGlyph.setCharacterSize(10);
        currentPlayerGlyph.setFillColor(uiTextColor);
        currentPlayerGlyph.setOutlineColor(uiTextColor);
        app.draw(currentPlayerGlyph);
    }
    {
        std::stringstream frameNumberString;
        frameNumberString << "Frame: " << drawnFrame.getFrameNumber();
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(frameNumberString.str());
        frameNumberGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) + 60);
        frameNumberGlyph.setCharacterSize(16);
        frameNumberGlyph.setFillColor(uiTextColor);
        frameNumberGlyph.setOutlineColor(uiTextColor);
        app.draw(frameNumberGlyph);
    }
    {
        std::stringstream timeString;
        timeString << "Time: " << (drawnFrame.getFrameNumber()*10/hg::FRAMERATE)/10. << "s";
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(timeString.str());
        frameNumberGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) + 20);
        frameNumberGlyph.setCharacterSize(16);
        frameNumberGlyph.setFillColor(uiTextColor);
        frameNumberGlyph.setOutlineColor(uiTextColor);
        app.draw(frameNumberGlyph);
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
}
}
}

