#include "GameDisplayHelpers.h"
#include "sfRenderTargetCanvas.h"
#include "GlobalConst.h"
#include <boost/range/algorithm/find_if.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <sstream>
#include "Maths.h"
namespace hg {

sf::Color const uiTextColor(100, 100, 200);

void DrawGlitzAndWall(
    hg::RenderWindow &target,
    hg::mt::std::vector<hg::Glitz> const &glitz,
    hg::Wall const &wall,
    hg::LevelResources const &resources,
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    sf::Image const &wallImage,
    sf::Image const &positionColoursImage)
{
    //Number by which all positions are be multiplied
    //to shrink or enlarge the level to the size of the
    //window.
    double scalingFactor(std::min(target.getSize().x*(100. - hg::UI_DIVIDE_X) / wall.roomWidth(), target.getSize().y*hg::UI_DIVIDE_Y / wall.roomHeight()));
    //double xGap = static_cast<float>(target.getSize().x / scalingFactor) - static_cast<float>(wall.roomWidth() / (100. - hg::UI_DIVIDE_X));
    //double yGap = static_cast<float>(target.getSize().y / scalingFactor) - static_cast<float>(target.getSize().y*hg::UI_DIVIDE_Y / 100.);
    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(
        static_cast<float>(-1.*target.getSize().x*hg::UI_DIVIDE_X / (100.*scalingFactor)),
        0.f,
        static_cast<float>(target.getSize().x / scalingFactor),
        static_cast<float>(target.getSize().y / scalingFactor)));
    target.setView(scaledView);

    hg::sfRenderTargetCanvas canvas(target.getRenderTarget(), audioPlayingState, audioGlitzManager, resources);
    hg::LayeredCanvas layeredCanvas(canvas);
    for (hg::Glitz const &particularGlitz : glitz) particularGlitz.display(layeredCanvas);
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);

    sf::Texture wallTex;
    wallTex.loadFromImage(wallImage);
    target.draw(sf::Sprite(wallTex));

    flusher.partialFlush(std::numeric_limits<int>::max());
    if (target.getInputState().isKeyPressed(sf::Keyboard::LShift)) {
        DrawColors(target, positionColoursImage);
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
        timeJumpGlyph.setPosition(20, static_cast<int>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y / 100) - 140);
        timeJumpGlyph.setCharacterSize(16);
        timeJumpGlyph.setFillColor(uiTextColor);
        timeJumpGlyph.setOutlineColor(uiTextColor);
        app.draw(timeJumpGlyph);
    }
    {
        std::stringstream timeReverses;
        timeReverses << (abilityCursor == Ability::TIME_REVERSE ? "-->" : "   ") << "2) timeReverses: " << mpickups[Ability::TIME_REVERSE];
        sf::Text timeReversesGlyph;
        timeReversesGlyph.setFont(*hg::defaultFont);
        timeReversesGlyph.setString(timeReverses.str());
        timeReversesGlyph.setPosition(20, static_cast<int>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y / 100) - 110);
        timeReversesGlyph.setCharacterSize(16);
        timeReversesGlyph.setFillColor(uiTextColor);
        timeReversesGlyph.setOutlineColor(uiTextColor);
        app.draw(timeReversesGlyph);
    }
    {
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == Ability::TIME_GUN ? "-->" : "   ") << "3) timeGuns: " << mpickups[Ability::TIME_GUN];
        sf::Text timeGunsGlyph;
        timeGunsGlyph.setFont(*hg::defaultFont);
        timeGunsGlyph.setString(timeGuns.str());
        timeGunsGlyph.setPosition(20, static_cast<int>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y / 100) - 80);
        timeGunsGlyph.setCharacterSize(16);
        timeGunsGlyph.setFillColor(uiTextColor);
        timeGunsGlyph.setOutlineColor(uiTextColor);
        app.draw(timeGunsGlyph);
    }
    {
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == Ability::TIME_PAUSE ? "-->" : "   ") << "4) timePauses: " << mpickups[Ability::TIME_PAUSE];
        sf::Text timePausesGlyph;
        timePausesGlyph.setFont(*hg::defaultFont);
        timePausesGlyph.setString(timeGuns.str());
        timePausesGlyph.setPosition(20, static_cast<int>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y / 100) - 50);
        timePausesGlyph.setCharacterSize(16);
        timePausesGlyph.setFillColor(uiTextColor);
        timePausesGlyph.setOutlineColor(uiTextColor);
        app.draw(timePausesGlyph);
    }
}

constexpr double clamp(double const val, double const min, double const max)
{
    return val < min ?
        min
        : val > max ? max : val;
}

sf::Color asColor(sf::Vector3<double> const &vec) {
    return sf::Color(static_cast<sf::Uint8>(clamp(vec.x, 0., 1.) * 255), static_cast<sf::Uint8>(clamp(vec.y, 0., 1.) * 255), static_cast<sf::Uint8>(clamp(vec.z, 0., 1.) * 255));
}


sf::Color guyPositionToColor(double xFrac, double yFrac) {
    static sf::Vector3<double> const posStart(1, 0, 0);
    static sf::Vector3<double> const xMax(0.5, 1, 0);
    static sf::Vector3<double> const yMax(0.5, 0, 1);

    static sf::Vector3<double> const xDif(xMax - posStart);
    static sf::Vector3<double> const yDif(yMax - posStart);

    return asColor(posStart + xDif*xFrac + yDif*yFrac);
}


void DrawColors(hg::RenderWindow &target, sf::Image const &positionColoursImage)
{
    sf::Texture tex;
    tex.loadFromImage(positionColoursImage);

    target.draw(sf::Sprite(tex));
}

void DrawTimelineContents(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    unsigned const height)
{
    static constexpr int boxLineHeight = 1;
    static constexpr int guyLineHeightStandard = 4;
    sf::Image timelineContents;
    timelineContents.create(static_cast<int>(std::round(target.getView().getSize().x)), height, sf::Color(0, 0, 0, 0));
    //TODO: This can become very slow on HiDPI displays (because the texture width is based on the window width in pixels)(?)
    //      It also looks bad, due to aliasing artefacts.
    //      Check; and reconsider the algorithm/implementation.

    //For example:
    //   * only redraw the changed frames; cache the texture between renders.
    //   * draw as colored lines with vertex shader and cached line textures??
    std::size_t const numberOfGuys(timeEngine.getReplayData().size() + 1);
    int const timelineLength(timeEngine.getTimelineLength());
    hg::UniverseID const universe(timeEngine.getTimelineLength());
    assert(numberOfGuys > 0);
    const int guyLineHeight = std::max(static_cast<int>(std::ceil(static_cast<double>(height) / numberOfGuys)), guyLineHeightStandard);
    for (int frameNumber = 0, end = timeEngine.getTimelineLength(); frameNumber != end; ++frameNumber) {
        hg::Frame const *const frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        assert(!isNullFrame(frame));
        int const left = static_cast<int>(frameNumber*target.getView().getSize().x / timelineLength);
        for (hg::GuyOutputInfo const &guy : frame->getView().getGuyInformation()) {
            std::size_t const top = static_cast<std::size_t>((height - guyLineHeight)*(guy.getIndex() / static_cast<double>(numberOfGuys)));

            double const xFrac = (guy.getX() - timeEngine.getWall().segmentSize()) / static_cast<double>(timeEngine.getWall().roomWidth()  - 2 * timeEngine.getWall().segmentSize());
            double const yFrac = (guy.getY() - timeEngine.getWall().segmentSize()) / static_cast<double>(timeEngine.getWall().roomHeight() - 2 * timeEngine.getWall().segmentSize());

            sf::Color const color(guyPositionToColor(xFrac, yFrac));

            std::size_t pos(top);
            for (std::size_t const bot(top + boxLineHeight); pos != bot; ++pos) {
                assert(pos <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
                auto const xPix = static_cast<unsigned int>(left);
                auto const yPix = static_cast<unsigned int>(pos);
                assert(xPix < timelineContents.getSize().x);
                assert(yPix < timelineContents.getSize().y);
                timelineContents.setPixel(
                    xPix, yPix,
                    !guy.getBoxCarrying() ?
                    color :
                    guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                    sf::Color(255, 0, 255)
                    : sf::Color(0, 255, 0));
            }
            for (std::size_t const bot(top + guyLineHeight); pos != bot; ++pos) {
                assert(pos <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
                auto const xPix = static_cast<unsigned int>(left);
                auto const yPix = static_cast<unsigned int>(pos);
                assert(xPix < timelineContents.getSize().x);
                assert(yPix < timelineContents.getSize().y);
                timelineContents.setPixel(xPix, yPix, color);
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
    //TODO: This can become slow on HiDPI displays, because it is determined by the width of the display in pixels(?)
    //It also looks bad; due to alisaing artefacts.
    //Come up with a better algorithm.
    std::vector<char> pixelsWhichHaveBeenDrawnIn(static_cast<std::size_t>(std::round(target.getView().getSize().x)));
    for (hg::FrameUpdateSet const &wave : waves) {
        for (hg::Frame *frame : wave) {
            if (frame) {
                auto pixelToDrawIn = static_cast<std::size_t>(
                    (static_cast<double>(getFrameNumber(frame)) / timelineLength)
                    *target.getView().getSize().x);
                assert(pixelToDrawIn < pixelsWhichHaveBeenDrawnIn.size());
                pixelsWhichHaveBeenDrawnIn[pixelToDrawIn] = true;
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
        bool inWaveRegion = false;
        int leftOfWaveRegion = 0;
        assert(pixelsWhichHaveBeenDrawnIn.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
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
                    sf::RectangleShape wavegroup(sf::Vector2f(static_cast<float>(i - leftOfWaveRegion), static_cast<float>(height)));
                    wavegroup.setPosition(static_cast<float>(leftOfWaveRegion), 10.f);
                    wavegroup.setFillColor(sf::Color(250, 0, 0));
                    target.draw(wavegroup);
                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            sf::RectangleShape wavegroup(sf::Vector2f(target.getView().getSize().x - leftOfWaveRegion, static_cast<float>(height)));
            wavegroup.setPosition(static_cast<float>(leftOfWaveRegion), 10.f);
            wavegroup.setFillColor(sf::Color(250, 0, 0));
            target.draw(wavegroup);
        }
    }
}


void DrawTicks(sf::RenderTarget &target, std::size_t const timelineLength) {
    for (std::size_t frameNo(0); frameNo < timelineLength; frameNo += 5 * 60) {
        float const left(static_cast<float>(frameNo / static_cast<double>(timelineLength)*target.getView().getSize().x));
        sf::RectangleShape tick(sf::Vector2f(1., 10.));
        tick.setFillColor(sf::Color(255, 255, 0));
        tick.setPosition(sf::Vector2f(left, 0.));
        target.draw(tick);
    }
}


void DrawPersonalTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    std::vector<Frame *> const &guyFrames) {

    //Horizontal Axis:
    // Guy Index
    //Vertical Axis:
    // Frame Index

    //Colour/Hat = Position/BoxCarrying

    //Waves/Active Frame TODO
    //Time Ticks TODO
    //Special display of dead guy frames? TODO
    auto const timelineLength(timeEngine.getTimelineLength());
    auto const topOfTimeline{380};
    auto const height{75};
    auto const actualGuyFrames{boost::make_iterator_range(guyFrames.begin(), guyFrames.end() - 1)};
    auto const guyFramesLength{boost::size(actualGuyFrames)};
    for(std::size_t i{0}; i != guyFramesLength; ++i) {
        auto const guyFrame{actualGuyFrames[i]};
        if (isNullFrame(guyFrame)) continue;

        auto const frameWidth{ float{target.getView().getSize().x / guyFramesLength }};
        auto const frameHorizontalPosition {float{i*frameWidth}};

        auto const frameHeight{ static_cast<float>(height / static_cast<double>(timelineLength))};
        auto const frameVerticalPosition{float{topOfTimeline+frameHeight*getFrameNumber(guyFrame)}};
        hg::GuyOutputInfo guy{*boost::find_if(guyFrame->getView().getGuyInformation(), [i](auto const& guyInfo) {return guyInfo.getIndex() == i;})};

        //TODO: Share this logic with DrawTimelineContents!
        double const xFrac = (guy.getX() - timeEngine.getWall().segmentSize()) / static_cast<double>(timeEngine.getWall().roomWidth() - 2 * timeEngine.getWall().segmentSize());
        double const yFrac = (guy.getY() - timeEngine.getWall().segmentSize()) / static_cast<double>(timeEngine.getWall().roomHeight() - 2 * timeEngine.getWall().segmentSize());

        sf::Color const frameColor(guyPositionToColor(xFrac, yFrac));
        sf::RectangleShape frameLine(sf::Vector2f(frameWidth, std::max(4.f,frameHeight)));
        frameLine.setPosition(frameHorizontalPosition, frameVerticalPosition);
        frameLine.setFillColor(frameColor);
        target.draw(frameLine);

        if (guy.getBoxCarrying()) {
            sf::Color const boxColor(guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                sf::Color(255, 0, 255)
                : sf::Color(0, 255, 0));
            sf::RectangleShape boxLine(sf::Vector2f(frameWidth, std::max(4.f, frameHeight)/4.f));
            boxLine.setPosition(frameHorizontalPosition, frameVerticalPosition);
            boxLine.setFillColor(boxColor);
            target.draw(boxLine);
        }

    }




}
void DrawTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID const playerFrame,
    hg::FrameID const timeCursor,
    int const timelineLength)
{
    unsigned int const height = 75;

    DrawTicks(target, timelineLength);

    DrawWaves(target, waves, timelineLength, height);

    if (playerFrame.isValidFrame()) {
        sf::RectangleShape playerLine(sf::Vector2f(3.f, static_cast<float>(height)));
        playerLine.setPosition(playerFrame.getFrameNumber()*target.getView().getSize().x / timelineLength, 10.f);
        playerLine.setFillColor(sf::Color(200, 200, 0));
        target.draw(playerLine);
    }
    if (timeCursor.isValidFrame()) {
        sf::Color const timeCursorColor(0, 0, 200);
        sf::RectangleShape timeCursorLine(sf::Vector2f(3.f, static_cast<float>(height)));
        float const timeCursorHorizontalPosition{ timeCursor.getFrameNumber()*target.getView().getSize().x / timelineLength };
        timeCursorLine.setPosition(timeCursorHorizontalPosition, 10.f);
        timeCursorLine.setFillColor(timeCursorColor);
        target.draw(timeCursorLine);

        {
            std::stringstream cursorTime;
            cursorTime << (timeCursor.getFrameNumber()*10 / hg::FRAMERATE)/10. << "s";
            sf::Text cursorTimeGlyph;
            cursorTimeGlyph.setFont(*hg::defaultFont);
            cursorTimeGlyph.setString(cursorTime.str());
            cursorTimeGlyph.setPosition(timeCursorHorizontalPosition-3.f, height + 20.f);
            cursorTimeGlyph.setCharacterSize(10);
            cursorTimeGlyph.setFillColor(timeCursorColor);
            cursorTimeGlyph.setOutlineColor(timeCursorColor);
            target.draw(cursorTimeGlyph);
        }
    }
    DrawTimelineContents(target, timeEngine, height);
}


void DrawInterfaceBorder(
    sf::RenderTarget &target)
{
    sf::Color const borderColor(0, 0, 0);
    sf::RectangleShape horizontalLine(sf::Vector2f(static_cast<float>(hg::WINDOW_DEFAULT_X), 3.f));
    horizontalLine.setPosition(0.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y / 100) - 1.5f);
    horizontalLine.setFillColor(borderColor);
    target.draw(horizontalLine);

    sf::RectangleShape verticalLine(sf::Vector2f(3.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y / 100)));
    verticalLine.setPosition(static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X / 100) - 1.5f, 0.f);
    verticalLine.setFillColor(borderColor);
    target.draw(verticalLine);
}

struct CompareIndicies {
    template<typename IndexableType>
    bool operator()(IndexableType const &l, IndexableType const &r) {
        return l.getIndex() < r.getIndex();
    }
};

hg::FrameID mousePosToFrameID(hg::RenderWindow const &app, hg::TimeEngine const &timeEngine) {
    int const timelineLength = timeEngine.getTimelineLength();
    double const mouseXFraction = app.getInputState().getMousePosition().x*1. / app.getSize().x;
    int mouseFrame(hg::flooredModulo(static_cast<int>(mouseXFraction*timelineLength), timelineLength));
    return hg::FrameID(mouseFrame, hg::UniverseID(timelineLength));
}
}
