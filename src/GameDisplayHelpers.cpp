#include "GameDisplayHelpers.h"
#include "sfRenderTargetCanvas.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <sstream>
#include "Maths.h"
namespace hg {


sf::Color const uiTextColor(100,100,200);

void DrawGlitzAndWall(
    hg::RenderWindow &target,
    hg::mt::std::vector<hg::Glitz> const &glitz,
    hg::Wall const &wall,
    hg::LevelResources const &resources,
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    sf::Image const &wallImage)
{
    target.clear(sf::Color(255,255,255));
    //Number by which all positions are be multiplied
    //to shrink or enlarge the level to the size of the
    //window.
    double scalingFactor(std::min(target.getSize().x*100./wall.roomWidth(), target.getSize().y*100./wall.roomHeight()));
    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(0.f, 0.f, target.getSize().x/scalingFactor, target.getSize().y/scalingFactor));
    target.setView(scaledView);
    hg::sfRenderTargetCanvas canvas(target.getRenderTarget(), audioPlayingState, audioGlitzManager, resources);
    hg::LayeredCanvas layeredCanvas(canvas);
    for (hg::Glitz const &particularGlitz: glitz) particularGlitz.display(layeredCanvas);
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);
    
    sf::Texture wallTex;
    wallTex.loadFromImage(wallImage);
    target.draw(sf::Sprite(wallTex));
    
    flusher.partialFlush(std::numeric_limits<int>::max());
    if (target.getInputState().isKeyPressed(sf::Keyboard::LShift)) {
        DrawColors(target, wall.roomWidth(), wall.roomHeight());
    }
    target.setView(oldView);
    
    canvas.flushFrame();
}

void drawInventory(
    hg::RenderWindow &app,
    hg::mt::std::map<hg::Ability, int> const &pickups,
    hg::Ability abilityCursor)
{
    hg::mt::std::map<hg::Ability, int> mpickups(pickups);
    {
        std::stringstream timeJump;
        timeJump << (abilityCursor == Ability::TIME_JUMP ? "-->" : "   ") << "1) timeJumps: " << mpickups[Ability::TIME_JUMP];
        sf::Text timeJumpGlyph;
        timeJumpGlyph.setFont(*hg::defaultFont);
        timeJumpGlyph.setString(timeJump.str());
        timeJumpGlyph.setPosition(500, 350);
        timeJumpGlyph.setCharacterSize(10.f);
        timeJumpGlyph.setColor(uiTextColor);
        app.draw(timeJumpGlyph);
    }
    {
        std::stringstream timeReverses;
        timeReverses << (abilityCursor == Ability::TIME_REVERSE ? "-->" : "   ") << "2) timeReverses: " << mpickups[Ability::TIME_REVERSE];
        sf::Text timeReversesGlyph;
        timeReversesGlyph.setFont(*hg::defaultFont);
        timeReversesGlyph.setString(timeReverses.str());
        timeReversesGlyph.setPosition(500, 370);
        timeReversesGlyph.setCharacterSize(10.f);
        timeReversesGlyph.setColor(uiTextColor);
        app.draw(timeReversesGlyph);
    }
    {
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == Ability::TIME_GUN ? "-->" : "   ") << "3) timeGuns: " << mpickups[Ability::TIME_GUN];
        sf::Text timeGunsGlyph;
        timeGunsGlyph.setFont(*hg::defaultFont);
        timeGunsGlyph.setString(timeGuns.str());
        timeGunsGlyph.setPosition(500, 390);
        timeGunsGlyph.setCharacterSize(10.f);
        timeGunsGlyph.setColor(uiTextColor);
        app.draw(timeGunsGlyph);
    }
    {
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == Ability::TIME_PAUSE ? "-->" : "   ") << "4) timePauses: " << mpickups[Ability::TIME_PAUSE];
        sf::Text timePausesGlyph;
        timePausesGlyph.setFont(*hg::defaultFont);
        timePausesGlyph.setString(timeGuns.str());
        timePausesGlyph.setPosition(500, 410);
        timePausesGlyph.setCharacterSize(10.f);
        timePausesGlyph.setColor(uiTextColor);
        app.draw(timePausesGlyph);
    }
}



sf::Color asColor(sf::Vector3<double>const &vec) {
    return sf::Color(vec.x, vec.y, vec.z);
}


sf::Color guyPositionToColor(double xFrac, double yFrac) {
    static sf::Vector3<double> const posStart(255,0,0);
    static sf::Vector3<double> const xMax(127,255,0);
    static sf::Vector3<double> const yMax(128,0,255);
    
    static sf::Vector3<double> const xDif(xMax - posStart);
    static sf::Vector3<double> const yDif(yMax - posStart);

    return asColor(posStart + xDif*xFrac + yDif*yFrac);
}


void DrawColors(hg::RenderWindow &target, int roomWidth, int roomHeight)
{
    sf::Image colors;
    colors.create(roomWidth/100, roomHeight/100, sf::Color(0, 0, 0, 0));
    for (int x(0); x != roomWidth/100; ++x) {
        for (int y(0); y != roomHeight/100; ++y) {
            sf::Color color(guyPositionToColor(x*100./roomWidth,y*100./roomHeight));
            color.a = 220;
            colors.setPixel(x, y, color);
        }
    }
    sf::Texture tex;
    tex.loadFromImage(colors);
    
    target.draw(sf::Sprite(tex));
}


void DrawTimelineContents(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    double height)
{
    sf::Image timelineContents;
    timelineContents.create(target.getView().getSize().x, height, sf::Color(0, 0, 0, 0));
    double const numberOfGuys(timeEngine.getReplayData().size()+1);
    double const timelineLength(timeEngine.getTimelineLength());
    hg::UniverseID const universe(timeEngine.getTimelineLength());
    
    for (int frameNumber = 0, end = timeEngine.getTimelineLength(); frameNumber != end; ++frameNumber) {
        hg::Frame const *const frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        for (hg::GuyOutputInfo const &guy: frame->getView().getGuyInformation()) {
            double left = frameNumber*target.getView().getSize().x/timelineLength;
            double top = (height-4)*guy.getIndex()/numberOfGuys;
            
            double xFrac = guy.getX()/static_cast<double>(timeEngine.getWall().roomWidth());
            double yFrac = guy.getY()/static_cast<double>(timeEngine.getWall().roomHeight());

            sf::Color const color(guyPositionToColor(xFrac, yFrac));

            int pos(top);
            for (int const end(top+1); pos != end; ++pos) {
                timelineContents.setPixel(
                    left, pos,
                    !guy.getBoxCarrying() ?
                        color :
                        guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                            sf::Color(255, 0, 255)
                          : sf::Color(0, 255, 0));
            }
            for (int const end(top+4); pos != end; ++pos) {
                timelineContents.setPixel(left, pos, color);
            }
        }
    }
    sf::Texture tex;
    tex.loadFromImage(timelineContents);
    sf::Sprite sprite(tex);
    sprite.setPosition(0.f, 10.f);
    target.draw(sprite);
}


void DrawWaves(
    sf::RenderTarget &target,
    hg::TimeEngine::FrameListList const &waves,
    int timelineLength,
    double height)
{
    std::vector<char> pixelsWhichHaveBeenDrawnIn(target.getView().getSize().x);
    for (hg::FrameUpdateSet const &wave: waves) {
        for (hg::Frame *frame: wave) {
            if (frame) {
                pixelsWhichHaveBeenDrawnIn[
                    static_cast<int>(
                        (static_cast<double>(getFrameNumber(frame))/timelineLength)
                        *target.getView().getSize().x)
                    ] = true;
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
        bool inWaveRegion = false;
        int leftOfWaveRegion = 0;
        for (int i = 0; i != static_cast<int>(pixelsWhichHaveBeenDrawnIn.size()); ++i) {
            bool pixelOn = pixelsWhichHaveBeenDrawnIn[i];
            if (pixelOn) {
                if (!inWaveRegion) {
                    leftOfWaveRegion = i;
                    inWaveRegion = true;
                }
            }
            else {
                if (inWaveRegion) {
                    sf::RectangleShape wavegroup(sf::Vector2f(i-leftOfWaveRegion, height));
                    wavegroup.setPosition(leftOfWaveRegion, 10.f);
                    wavegroup.setFillColor(sf::Color(250,0,0));
                    target.draw(wavegroup);
                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            sf::RectangleShape wavegroup(sf::Vector2f(target.getView().getSize().x-leftOfWaveRegion, height));
            wavegroup.setPosition(leftOfWaveRegion, 10.f);
            wavegroup.setFillColor(sf::Color(250,0,0));
            target.draw(wavegroup);
        }
    }
}


void DrawTicks(sf::RenderTarget &target, std::size_t const timelineLength) {
    for (std::size_t frameNo(0); frameNo < timelineLength; frameNo += 5*60) {
        float left(frameNo/static_cast<double>(timelineLength)*target.getView().getSize().x);
        sf::RectangleShape tick(sf::Vector2f(1., 10.));
        tick.setFillColor(sf::Color(255,255,0));
        tick.setPosition(sf::Vector2f(left, 0.));
        target.draw(tick);
    }
}


void DrawTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID playerFrame,
    hg::FrameID timeCursor,
    int timelineLength)
{
    double height = 75.;
    
    DrawTicks(target, timelineLength);
    
    DrawWaves(target, waves, timelineLength, height);
    
    if (playerFrame.isValidFrame()) {
        sf::RectangleShape playerLine(sf::Vector2f(3, height));
        playerLine.setPosition(playerFrame.getFrameNumber()*target.getView().getSize().x/timelineLength, 10.f);
        playerLine.setFillColor(sf::Color(200,200,0));
        target.draw(playerLine);
    }
    if (timeCursor.isValidFrame()) {
        sf::RectangleShape timeCursorLine(sf::Vector2f(3, height));
        timeCursorLine.setPosition(timeCursor.getFrameNumber()*target.getView().getSize().x/timelineLength, 10.f);
        timeCursorLine.setFillColor(sf::Color(0,0,200));
        target.draw(timeCursorLine);
    }
    DrawTimelineContents(target, timeEngine, height);
}


struct CompareIndicies {
    template<typename IndexableType>
    bool operator()(IndexableType const &l, IndexableType const &r) {
        return l.getIndex() < r.getIndex();
    }
};

hg::FrameID mousePosToFrameID(hg::RenderWindow const &app, hg::TimeEngine const &timeEngine) {
    int const timelineLength = timeEngine.getTimelineLength();
    double const mouseXFraction = app.getInputState().getMousePosition().x*1./app.getSize().x;
    int mouseFrame(hg::flooredModulo(static_cast<int>(mouseXFraction*timelineLength), timelineLength));
    return hg::FrameID(mouseFrame, hg::UniverseID(timelineLength));
}




}
