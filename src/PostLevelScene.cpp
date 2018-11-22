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
        AudioPlayingState &audioPlayingState,
        AudioGlitzManager &audioGlitzManager,
        hg::Inertia &inertia,
        hg::LevelResources const &resources,
        sf::Image const &wallImage,
        sf::Image const &positionColoursImage);
}
void run_post_level_scene(
    hg::RenderWindow &window,
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
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                switch (event.type) {
                    //Window Closed
                    case sf::Event::Closed:
                        window.close();
                        throw WindowClosed_exception{};
                    case sf::Event::KeyPressed:
                    switch (event.key.code) {
                      //Esc Pressed
                      case sf::Keyboard::Escape:
                        return;
                      //Save Replay
                      case sf::Keyboard::K:
                        saveReplay("replay", finalLevel.timeEngine.getReplayData());
                      break;
                      //Restart Replay
                    
                      //Pause Relative Replay
                    
                      //Resume Relative Replay
                    
                      default:
                        break;
                    }
                    
                   
                    break;
                default:
                    break;
                }
            }
            //Inertia Forwards/Backwards
            if (window.getInputState().isKeyPressed(sf::Keyboard::Period)) {
                inertia.save(mousePosToFrameID(window, timeEngine), TimeDirection::FORWARDS);
            }
            if (window.getInputState().isKeyPressed(sf::Keyboard::Comma)) {
                inertia.save(mousePosToFrameID(window, timeEngine), TimeDirection::REVERSE);
            }
            if (window.getInputState().isKeyPressed(sf::Keyboard::Slash)) {
                inertia.reset();
            }
            runStep(timeEngine, window, audioPlayingState, audioGlitzManager, inertia, levelResources, wallImage, positionColoursImage);
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
            if (window.getInputState().isKeyPressed(sf::Keyboard::F)) {
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
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    hg::Inertia &inertia,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    sf::Image const &positionColoursImage)
{
    app.clear(sf::Color(255, 255, 255));
    hg::FrameID drawnFrame;

    if (app.getInputState().isKeyPressed(sf::Keyboard::LControl)) {
        drawnFrame =
              hg::FrameID(
                abs(
                  static_cast<int>(
                    hg::flooredModulo(static_cast<long>((sf::Mouse::getPosition(app.getWindow()).x
                     * static_cast<long>(timeEngine.getTimelineLength())
                     / app.getSize().x))
                    , static_cast<long>(timeEngine.getTimelineLength())))),
                hg::UniverseID(timeEngine.getTimelineLength()));
        hg::Frame const *frame(timeEngine.getFrame(drawnFrame));
        DrawGlitzAndWall(app,
            getGlitzForDirection(frame->getView(), TimeDirection::FORWARDS),
            timeEngine.getWall(),
            resources,
            audioPlayingState,
            audioGlitzManager,
            wallImage,
            positionColoursImage);
    }
    else {
        inertia.run();
        hg::FrameID const inertialFrame(inertia.getFrame());
        if (inertialFrame.isValidFrame()) {
            drawnFrame = inertialFrame;
            hg::Frame const *frame(timeEngine.getFrame(inertialFrame));
            DrawGlitzAndWall(app,
                getGlitzForDirection(frame->getView(), inertia.getTimeDirection()),
                timeEngine.getWall(),
                resources,
                audioPlayingState,
                audioGlitzManager,
                wallImage,
                positionColoursImage);
        }
        else {
            drawnFrame =
              hg::FrameID(
                abs(
                  static_cast<int>(
                    hg::flooredModulo(static_cast<long>((sf::Mouse::getPosition(app.getWindow()).x
                     * static_cast<long>(timeEngine.getTimelineLength())
                     / app.getSize().x))
                    , static_cast<long>(timeEngine.getTimelineLength())))),
                hg::UniverseID(timeEngine.getTimelineLength()));
            hg::Frame const *frame(timeEngine.getFrame(drawnFrame));
            DrawGlitzAndWall(app,
                getGlitzForDirection(frame->getView(), TimeDirection::FORWARDS),
                timeEngine.getWall(),
                resources,
                audioPlayingState,
                audioGlitzManager,
                wallImage,
                positionColoursImage);
        }
    }
    DrawTimeline(app.getRenderTarget(), timeEngine, TimeEngine::FrameListList{}, drawnFrame, timeEngine.getReplayData().back().getGuyInput().getTimeCursor(), timeEngine.getTimelineLength());
    /*DrawPersonalTimeline( //TODO (Probably needs to be tweaked a bit to be suitable for PostLevelScene)
        app.getRenderTarget(),
        timeEngine,
        waveInfo.guyFrames);
        */
    DrawInterfaceBorder(app.getRenderTarget());

    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << timeEngine.getReplayData().size() - 1;
        sf::Text currentPlayerGlyph;
        currentPlayerGlyph.setFont(*hg::defaultFont);
        currentPlayerGlyph.setString(currentPlayerIndex.str());
        currentPlayerGlyph.setPosition(580, 433);
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
        frameNumberGlyph.setPosition(580, 445);
        frameNumberGlyph.setCharacterSize(8);
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
        frameNumberGlyph.setPosition(580, 457);
        frameNumberGlyph.setCharacterSize(8);
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

