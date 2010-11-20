#include "PlayingGameState.h"
#include "PlayerVictoryException.h"
#include "GameEngine.h"
#include "Level.h"
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/String.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <iostream>
#define foreach BOOST_FOREACH
namespace hg {
PlayingGameState::PlayingGameState(::std::auto_ptr<State>& currentState, GameEngine& engine, const Level& level) :
currentState_(currentState),
level_(level),
engine_(engine),
timeEngine_(level),
levelDrawer_(level),
input_()
{
}
void PlayingGameState::init()
{
    assert(this == currentState_.get() && "pretty sure it's being used wrong if this is not the case at any stage after construction");
    engine_.window.UseVerticalSync(true);
    engine_.window.SetFramerateLimit(60);
}
static void drawTimeline(sf::RenderTarget& target, TimeEngine::FrameListList& waves, NewFrameID& playerFrame)
{
    bool pixelsWhichHaveBeenDrawnIn[640] = {false};
    foreach(const FrameUpdateSet& lists, waves) {
        foreach (NewFrameID frame, lists) {
            if (frame.isValidFrame()) {
                if (!pixelsWhichHaveBeenDrawnIn[static_cast<unsigned int> ((frame.frame()/10800.f)*640)]) {
                    target.Draw(sf::Shape::Rectangle((frame.frame()/10800.f)*640,
                                                10,
                                                (frame.frame()/10800.f)*640+1,
                                                25,
                                                sf::Color(250,0,0)));
                    pixelsWhichHaveBeenDrawnIn[static_cast<int> ((frame.frame()/10800.f)*640)] = true;
                }
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
    }
    if(playerFrame.isValidFrame()) {
        target.Draw(sf::Shape::Rectangle((playerFrame.frame()/10800.f)*640-1,
                                             10,
                                             (playerFrame.frame()/10800.f)*640+2,
                                             25,
                                             sf::Color(200,200,0)));
    }
}


void PlayingGameState::update()
{
    //Check for window resizes, ctrl/cmd-q etc...
    //Main game logic:
    // get input
    // push input into time engine
    // draw results
    //Very very basic for now, but no worse than the other version.
    //this new system is however much more extendable
    sf::Event event;
    while (engine_.window.GetEvent(event))
    {
        switch (event.Type) {
            case sf::Event::Closed:
            //Seems like this has a bit too much power. Should look into a more abstract interface
                engine_.stateManager.popAll();
                return;
            default:
            //should also check for window resizes and so on, but that can wait
                break;
        }
    }
    #if 0
    if (app.GetInput().IsKeyDown(sf::Key::S)) {
        const vector<InputList> replay(timeEngine.getReplayData());
        std::ofstream ofs("replay");
        {   
            boost::archive::xml_oarchive out(ofs);
            out << BOOST_SERIALIZATION_NVP(replay);
        }
    }
    #endif
    input_.updateState(engine_.window.GetInput());
    using boost::tuple;
    //cout << "called from main" << endl;
    try{
        tuple<NewFrameID, NewFrameID, TimeEngine::FrameListList, TimeDirection> waveInfo(timeEngine_.runToNextPlayerFrame(input_.AsInputList()));
        if (waveInfo.get<0>().isValidFrame()) {
            levelDrawer_.draw
            (
             engine_.window, 
             timeEngine_.getPostPhysics
             (
              waveInfo.get<0>(),
              waveInfo.get<1>().universe().pauseDepth() == waveInfo.get<0>().universe().pauseDepth()
              ?
              PauseInitiatorID(pauseinitiatortype::INVALID,0,0)
              :
              waveInfo.get<1>().universe().initiatorID()
              ),
             waveInfo.get<3>()
             );
        }
        else {
            levelDrawer_.draw(engine_.window, timeEngine_.getPostPhysics(NewFrameID(abs((engine_.window.GetInput().GetMouseX()*level_.timeLineLength/640)%level_.timeLineLength),level_.timeLineLength),PauseInitiatorID(pauseinitiatortype::INVALID,0,0)), waveInfo.get<3>());
        }
        drawTimeline(engine_.window, waveInfo.get<2>(), waveInfo.get<0>());
    }
    catch (PlayerVictoryException& playerWon) {
        std::cout << "Congratulations, a winner is you!\n";
        currentState_.release();
        return;
    }
    
    {
        std::stringstream fpsstring;
        fpsstring << (1./engine_.window.GetFrameTime());
        sf::String fpsglyph(fpsstring.str());
        fpsglyph.SetPosition(600, 465);
        fpsglyph.SetSize(8.f);
        engine_.window.Draw(fpsglyph);
    }
    engine_.window.Display();
}
}
