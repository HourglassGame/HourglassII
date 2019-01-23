#include "GameDisplayHelpers.h"
#include "sfRenderTargetCanvas.h"
#include "VulkanCanvas.h"
#include "GlobalConst.h"
#include <boost/range/algorithm/find_if.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <sstream>
#include "Maths.h"
namespace hg {

sf::Color const uiTextColor(100, 100, 200);


void PlayAudioGlitz(
    hg::mt::std::vector<hg::Glitz> const &glitz,
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    int const guyIndex)
{
    hg::sfRenderTargetAudioCanvas canvas(audioPlayingState, audioGlitzManager);

    hg::LayeredCanvas layeredCanvas(canvas);
    for (hg::Glitz const &particularGlitz : glitz)
    {
        particularGlitz.display(layeredCanvas, guyIndex);
    }
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(std::numeric_limits<int>::max());

    canvas.flushFrame();
}
void DrawVisualGlitzAndWall(
    sf::RenderTarget &target,
    hg::VulkanEngine &eng,
    hg::mt::std::vector<hg::Glitz> const &glitz,
    hg::Wall const &wall,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    sf::Image const &positionColoursImage,
    int const guyIndex,
    bool const drawPositionColours)
{
    sf::View const oldView(target.getView());
    {
        //Window Aspect Ratio to Viewport Aspect Ratio conversion rate:
        //Currently the ViewPort is the upper right portion of the window.
        //UI_DIVIDE_X and UI_DIVIDE_Y are the locations of the 2 lines that divide
        //the window into quadrants.
        double const ViewportWidthRatio = 1. - hg::UI_DIVIDE_X;
        double const ViewportHeightRatio = hg::UI_DIVIDE_Y;

        //Viewport Aspect Ratio is
        //VWidth : VHeight
        double const VWidth = target.getSize().x*ViewportWidthRatio;
        double const VHeight = target.getSize().y*ViewportHeightRatio;

        //Level Aspect Ratio is:
        //LWidth : LHeight
        //Divide by 100. since HG object positions/sizes are fixed point divided by 100
        double const LWidth = wall.roomWidth()/100.;
        double const LHeight = wall.roomHeight()/100.;

        //World View:
        //The view of the level should:
        // Have the same aspect ratio as the ViewPort
        // Exactly fit the level along one axis
        // Be larger than or equal in size to the level along the other axis,
        // and with a <= 0 initial position, such
        // that the level is centered in the view.
        double const xScale = VWidth / LWidth;
        double const yScale = VHeight / LHeight;
        double const scalingFactor(std::min(xScale, yScale));
        double const xFill = scalingFactor / xScale;
        double const yFill = scalingFactor / yScale;

        double const worldViewWidth = LWidth/xFill;
        double const worldViewHeight = LHeight/yFill;

        double const worldViewXPos = -1.*worldViewWidth*((1. - xFill) / 2.);
        double const worldViewYPos = -1.*worldViewHeight*((1. - yFill) / 2.);

        //Have the same aspect ratio as the ViewPort
        assert(essentiallyEqual(worldViewHeight/worldViewWidth, VHeight/VWidth, 0.00001));

        // Exactly fit the level along one axis
        assert(worldViewWidth == LWidth || worldViewHeight == LHeight);
        // Be larger than or equal in size to the level along the other axis,
        assert(worldViewHeight >= LHeight);
        assert(worldViewWidth >= LWidth);
        // With a <= initial position
        assert(worldViewXPos <= 0);
        assert(worldViewYPos <= 0);
        // such that the level is centered in the view
        assert(essentiallyEqual(worldViewXPos + worldViewWidth/2., LWidth/2., 0.00001));
        assert(essentiallyEqual(worldViewYPos + worldViewHeight/2., LHeight/2., 0.00001));

        sf::View scaledView(sf::FloatRect(
            worldViewXPos,
            worldViewYPos,
            worldViewWidth,
            worldViewHeight));

        //Game view is top-right quadrant
        scaledView.setViewport(
                sf::FloatRect(
                    static_cast<float>(hg::UI_DIVIDE_X),
                    0.f,
                    static_cast<float>(ViewportWidthRatio),
                    static_cast<float>(ViewportHeightRatio)));

        target.setView(scaledView);
    }
    hg::sfRenderTargetVisualCanvas sfRTcanvas(target, resources);
    hg::VulkanCanvas vkCanvas;
    hg::PairCanvas canvas(sfRTcanvas, vkCanvas);
    hg::LayeredCanvas layeredCanvas(canvas);
    for (hg::Glitz const &particularGlitz : glitz)
    {
        particularGlitz.display(layeredCanvas, guyIndex);
    }
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);

    sf::Texture wallTex;
    wallTex.loadFromImage(wallImage);
    target.draw(sf::Sprite(wallTex));

    flusher.partialFlush(std::numeric_limits<int>::max());
    if (drawPositionColours) {
        DrawColors(target, positionColoursImage);
    }
    target.setView(oldView);

    canvas.flushFrame();
    /*
    auto const &verts{vkCanvas.vertices};
    std::vector<hg::vec2<float>> projectedVerts;
    projectedVerts.reserve(verts.size());

    for(auto const &vert : verts) {
        auto const applyProj{[](float const in) {
            return in/2000.f;
        }};
        auto const projected{target.getView().getTransform().transformPoint(std::get<0>(vert), std::get<1>(vert))};
        projectedVerts.push_back(
            vec2<float>{
                projected.x,
                -projected.y
            }
        );
    }
    */
    //eng.drawFrame(projectedVerts);
}

void drawInventory(
    sf::RenderTarget &app,
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
        timeJumpGlyph.setPosition(20, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 140);
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
        timeReversesGlyph.setPosition(20, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 110);
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
        timeGunsGlyph.setPosition(20, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 80);
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
        timePausesGlyph.setPosition(20, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 50);
        timePausesGlyph.setCharacterSize(16);
        timePausesGlyph.setFillColor(uiTextColor);
        timePausesGlyph.setOutlineColor(uiTextColor);
        app.draw(timePausesGlyph);
    }
}

sf::Color asColor(sf::Vector3<double> const &vec) {
    return sf::Color(static_cast<sf::Uint8>(clamp(0., vec.x, 1.) * 255), static_cast<sf::Uint8>(clamp(0., vec.y, 1.) * 255), static_cast<sf::Uint8>(clamp(0., vec.z, 1.) * 255));
}


sf::Color guyPositionToColor(double xFrac, double yFrac) {
    static sf::Vector3<double> const posStart(1, 0, 0);
    static sf::Vector3<double> const xMax(0.5, 1, 0);
    static sf::Vector3<double> const yMax(0.5, 0, 1);

    static sf::Vector3<double> const xDif(xMax - posStart);
    static sf::Vector3<double> const yDif(yMax - posStart);

    return asColor(posStart + xDif*xFrac + yDif*yFrac);
}


void DrawColors(sf::RenderTarget &target, sf::Image const &positionColoursImage)
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
    auto const timelineContentsWidth{std::round(target.getView().getSize().x)};
    sf::Image timelineContents;
    timelineContents.create(static_cast<int>(timelineContentsWidth), height, sf::Color(0, 0, 0, 0));
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
        int const left = static_cast<int>(frameNumber*timelineContentsWidth / timelineLength);
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
    auto const waveDisplayWidth{std::round(target.getView().getSize().x)};
    std::vector<char> pixelsWhichHaveBeenDrawnIn(static_cast<std::size_t>(waveDisplayWidth));
    for (hg::FrameUpdateSet const &wave : waves) {
        for (hg::Frame *frame : wave) {
            if (frame) {
                auto pixelToDrawIn = static_cast<std::size_t>(
                    (static_cast<double>(getFrameNumber(frame)) / timelineLength)
                    *waveDisplayWidth);
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
    for (std::size_t frameNo(0); frameNo < timelineLength; frameNo += 5 * FRAMERATE) {
        float const left(static_cast<float>(frameNo / static_cast<double>(timelineLength)*target.getView().getSize().x));
        sf::RectangleShape tick(sf::Vector2f(2., 10.));
        tick.setFillColor(sf::Color(0, 0, 0));
        tick.setPosition(sf::Vector2f(left - 1.f, 0.));
        target.draw(tick);
    }
}


void DrawPersonalTimeline2(
    sf::RenderTarget &target,
    Wall const &wall,
    std::size_t const relativeGuyIndex,
    std::vector<std::optional<GuyFrameData>> const &guyFrames,
    std::vector<GuyInput> const &guyInput,
    std::size_t const minTimelineLength)
{

    auto const padding{ 3.f };
    auto const bottomSpace{ 9.f };
    auto const minFrameHeight{ 4.f };
    auto const height{ static_cast<float>(hg::P_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))*hg::WINDOW_DEFAULT_Y - 2.f*padding - bottomSpace };
    //Horizontal Axis:
    // Guy Index
    //Vertical Axis:
    // Frame Index

    float left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
    float right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
    float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
    float bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::P_TIME_Y + hg::P_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));

    sf::Color const borderColor(100, 100, 100);
    sf::RectangleShape horizontalLine(sf::Vector2f(right - left, 3.f));
    horizontalLine.setFillColor(borderColor);
    horizontalLine.setPosition(left, top - 1.5f);
    target.draw(horizontalLine);
    horizontalLine.setPosition(left, bot - 1.5f);
    target.draw(horizontalLine);

    sf::RectangleShape verticalLine(sf::Vector2f(3.f, bot - top + 3.f));
    verticalLine.setFillColor(borderColor);
    verticalLine.setPosition(left - 3.f, top - 1.5f);
    target.draw(verticalLine);
    verticalLine.setPosition(right, top - 1.5f);
    target.draw(verticalLine);

    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(
        0.f,
        0.f,
        target.getSize().x*static_cast<float>((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X),
        height + 2.f*padding + bottomSpace));
    scaledView.setViewport(sf::FloatRect(
        static_cast<float>(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X),
        static_cast<float>(hg::UI_DIVIDE_Y) + static_cast<float>(hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)),
        1.f - static_cast<float>(hg::UI_DIVIDE_X + 2.*hg::TIMELINE_PAD_X),
        static_cast<float>(hg::P_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))));
    target.setView(scaledView);

    //Colour/Hat = Position/BoxCarrying
    //Waves/Active Frame TODO
    //Time Ticks TODO
    //Special display of dead guy frames? TODO
    std::size_t skipInputFrames = 0;
    auto const actualGuyFrames{ boost::make_iterator_range(guyFrames.begin(), guyFrames.end() - 1) };
    auto const guyFramesLength{ boost::size(actualGuyFrames) };
    std::size_t const timelineLength{ std::max(minTimelineLength, guyFramesLength) };
    std::size_t const frameInc = static_cast<std::size_t>(std::max(1, static_cast<int>(std::floor(timelineLength / target.getView().getSize().x))));
    for (std::size_t i{ 0 }; i < guyFramesLength; i += frameInc) {
        auto const frameWidth{ float{target.getView().getSize().x * frameInc / timelineLength } };
        auto const frameHorizontalPosition{ float{i*target.getView().getSize().x / timelineLength} };
        auto const frameHeight{ static_cast<float>(height / static_cast<double>(timelineLength)) };

        if (skipInputFrames > 0)
        {
            --skipInputFrames;
        }
        else if (guyInput[i].getActionTaken())
        {
            sf::RectangleShape inputLine(sf::Vector2f(std::max(frameWidth, 3.f), bottomSpace - minFrameHeight - 1.f + padding / 2.f));
            if (guyInput[i].getPortalUsed())
            {
                inputLine.setFillColor(sf::Color(50, 255, 50));
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else if (guyInput[i].getAbilityUsed())
            {
                inputLine.setFillColor(sf::Color(50, 50, 255));
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else if (guyInput[i].getDown() || guyInput[i].getBoxLeft() || guyInput[i].getBoxRight())
            {
                inputLine.setFillColor(sf::Color(0, 0, 0));
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else
            {
                inputLine.setFillColor(sf::Color(120, 120, 120));
            }
            inputLine.setPosition(frameHorizontalPosition, padding + height + frameHeight + minFrameHeight + 1.f);
            target.draw(inputLine);
        }

        auto const guyFrame{ actualGuyFrames[i] };
        if (!guyFrame) continue;

        auto const frameVerticalPosition{ float{padding + frameHeight * guyFrame->frameNumber} };
        hg::GuyOutputInfo guy{guyFrame->guyOutputInfo};

        //TODO: Share this logic with DrawTimelineContents!
        double const xFrac = (guy.getX() - wall.segmentSize()) / static_cast<double>(wall.roomWidth() - 2 * wall.segmentSize());
        double const yFrac = (guy.getY() - wall.segmentSize()) / static_cast<double>(wall.roomHeight() - 2 * wall.segmentSize());

        sf::Color const frameColor(guyPositionToColor(xFrac, yFrac));
        sf::RectangleShape frameLine(sf::Vector2f(frameWidth, std::max(minFrameHeight, frameHeight)));
        frameLine.setPosition(frameHorizontalPosition, frameVerticalPosition);
        frameLine.setFillColor(frameColor);
        target.draw(frameLine);

        if (guy.getBoxCarrying()) {
            sf::Color const boxColor(guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                sf::Color(255, 0, 255)
                : sf::Color(0, 255, 0));
            sf::RectangleShape boxLine(sf::Vector2f(frameWidth, std::max(4.f, frameHeight) / 4.f));
            boxLine.setPosition(frameHorizontalPosition, frameVerticalPosition);
            boxLine.setFillColor(boxColor);
            target.draw(boxLine);
        }
    }

    sf::RectangleShape playerLine(sf::Vector2f(3.f, static_cast<float>(height + bottomSpace)));
    playerLine.setPosition((guyFramesLength - relativeGuyIndex)*target.getView().getSize().x / timelineLength, padding);
    playerLine.setFillColor(sf::Color(200, 200, 0));
    target.draw(playerLine);

    target.setView(oldView);
}

void DrawPersonalTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    std::size_t const relativeGuyIndex,
    std::vector<Frame *> const &guyFrames,
    std::vector<GuyInput> const &guyInput,
    std::size_t const minTimelineLength) {

    auto const padding{ 3.f };
    auto const bottomSpace{ 9.f };
    auto const minFrameHeight{ 4.f };
    auto const height{ static_cast<float>(hg::P_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))*hg::WINDOW_DEFAULT_Y - 2.f*padding - bottomSpace };
    //Horizontal Axis:
    // Guy Index
    //Vertical Axis:
    // Frame Index

    float left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
    float right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
    float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
    float bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::P_TIME_Y + hg::P_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));

    sf::Color const borderColor(100, 100, 100);
    sf::RectangleShape horizontalLine(sf::Vector2f(right - left, 3.f));
    horizontalLine.setFillColor(borderColor);
    horizontalLine.setPosition(left, top - 1.5f);
    target.draw(horizontalLine);
    horizontalLine.setPosition(left, bot - 1.5f);
    target.draw(horizontalLine);

    sf::RectangleShape verticalLine(sf::Vector2f(3.f, bot - top + 3.f));
    verticalLine.setFillColor(borderColor);
    verticalLine.setPosition(left - 3.f, top - 1.5f);
    target.draw(verticalLine);
    verticalLine.setPosition(right, top - 1.5f);
    target.draw(verticalLine);

    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(
        0.f,
        0.f,
        target.getSize().x*static_cast<float>((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X),
        height + 2.f*padding + bottomSpace));
    scaledView.setViewport(sf::FloatRect(
        static_cast<float>(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X),
        static_cast<float>(hg::UI_DIVIDE_Y) + static_cast<float>(hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)),
        1.f - static_cast<float>(hg::UI_DIVIDE_X + 2.*hg::TIMELINE_PAD_X),
        static_cast<float>(hg::P_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))));
    target.setView(scaledView);

    //Colour/Hat = Position/BoxCarrying
    //Waves/Active Frame TODO
    //Time Ticks TODO
    //Special display of dead guy frames? TODO
    std::size_t skipInputFrames = 0;
    auto const actualGuyFrames{boost::make_iterator_range(guyFrames.begin(), guyFrames.end() - 1)};
    auto const guyFramesLength{boost::size(actualGuyFrames)};
    std::size_t const timelineLength{ std::max(minTimelineLength, guyFramesLength) };
    std::size_t const frameInc = static_cast<std::size_t>(std::max(1, static_cast<int>(std::floor(timelineLength / target.getView().getSize().x))));
    for(std::size_t i{0}; i < guyFramesLength; i += frameInc) {
        auto const frameWidth{ float{target.getView().getSize().x * frameInc / timelineLength } };
        auto const frameHorizontalPosition{ float{i*target.getView().getSize().x / timelineLength} };
        auto const frameHeight{ static_cast<float>(height / static_cast<double>(timelineLength)) };
        
        if (skipInputFrames > 0)
        {
            --skipInputFrames;
        }
        else if (guyInput[i].getActionTaken())
        {
            sf::RectangleShape inputLine(sf::Vector2f(std::max(frameWidth, 3.f), bottomSpace - minFrameHeight - 1.f + padding / 2.f));
            if (guyInput[i].getPortalUsed())
            {
                inputLine.setFillColor(sf::Color(50, 255, 50));
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else if (guyInput[i].getAbilityUsed())
            {
                inputLine.setFillColor(sf::Color(50, 50, 255));
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else if (guyInput[i].getDown() || guyInput[i].getBoxLeft() || guyInput[i].getBoxRight())
            {
                inputLine.setFillColor(sf::Color(0, 0, 0));
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else
            {
                inputLine.setFillColor(sf::Color(120, 120, 120));
            }
            inputLine.setPosition(frameHorizontalPosition, padding + height + frameHeight + minFrameHeight + 1.f);
            target.draw(inputLine);
        }

        auto const guyFrame{ actualGuyFrames[i] };
        if (isNullFrame(guyFrame)) continue;

        auto const frameVerticalPosition{float{padding + frameHeight*getFrameNumber(guyFrame)}};
        hg::GuyOutputInfo guy{*boost::find_if(guyFrame->getView().getGuyInformation(), [i](auto const& guyInfo) {return guyInfo.getIndex() == i;})};

        //TODO: Share this logic with DrawTimelineContents!
        double const xFrac = (guy.getX() - timeEngine.getWall().segmentSize()) / static_cast<double>(timeEngine.getWall().roomWidth() - 2 * timeEngine.getWall().segmentSize());
        double const yFrac = (guy.getY() - timeEngine.getWall().segmentSize()) / static_cast<double>(timeEngine.getWall().roomHeight() - 2 * timeEngine.getWall().segmentSize());

        sf::Color const frameColor(guyPositionToColor(xFrac, yFrac));
        sf::RectangleShape frameLine(sf::Vector2f(frameWidth, std::max(minFrameHeight,frameHeight)));
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

    sf::RectangleShape playerLine(sf::Vector2f(3.f, static_cast<float>(height + bottomSpace)));
    playerLine.setPosition((guyFramesLength - relativeGuyIndex)*target.getView().getSize().x / timelineLength, padding);
    playerLine.setFillColor(sf::Color(200, 200, 0));
    target.draw(playerLine);

    target.setView(oldView);
}

void DrawTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine const &timeEngine,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID const playerFrame,
    hg::FrameID const timeCursor,
    int const timelineLength)
{
    float left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
    float right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
    float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
    float bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::G_TIME_Y + hg::G_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));

    sf::Color const borderColor(100, 100, 100);
    sf::RectangleShape horizontalLine(sf::Vector2f(right - left, 3.f));
    horizontalLine.setFillColor(borderColor);
    horizontalLine.setPosition(left, top - 1.5f);
    target.draw(horizontalLine);
    horizontalLine.setPosition(left, bot - 1.5f);
    target.draw(horizontalLine);

    sf::RectangleShape verticalLine(sf::Vector2f(3.f, bot - top + 3.f));
    verticalLine.setFillColor(borderColor);
    verticalLine.setPosition(left - 3.f, top - 1.5f);
    target.draw(verticalLine);
    verticalLine.setPosition(right, top - 1.5f);
    target.draw(verticalLine);

    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(
        0.f,
        0.f,
        target.getSize().x*static_cast<float>((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X),
        85.f));
    scaledView.setViewport(sf::FloatRect(
        static_cast<float>(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X),
        static_cast<float>(hg::UI_DIVIDE_Y) + static_cast<float>(hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y)),
        1.f - static_cast<float>(hg::UI_DIVIDE_X + 2.*hg::TIMELINE_PAD_X),
        static_cast<float>(hg::G_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))));
    target.setView(scaledView);

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

    target.setView(oldView);
}


void DrawTimelineContents2(
    sf::RenderTarget &target,
    unsigned const height,
    float const width,
    std::size_t const timelineLength,
    std::vector<std::optional<GuyFrameData>> const &guyFrames,
    Wall const &wall)
{
    static constexpr int boxLineHeight = 1;
    static constexpr int guyLineHeightStandard = 4;
    auto const timelineContentsWidth{ std::round(width) };
    sf::Image timelineContents;
    timelineContents.create(static_cast<int>(timelineContentsWidth), height, sf::Color(0, 0, 0, 0));
    //TODO: This can become very slow on HiDPI displays (because the texture width is based on the window width in pixels)(?)
    //      It also looks bad, due to aliasing artefacts.
    //      Check; and reconsider the algorithm/implementation.

    //For example:
    //   * only redraw the changed frames; cache the texture between renders.
    //   * draw as colored lines with vertex shader and cached line textures??
    //std::size_t const numberOfGuys(timeEngine.getReplayData().size() + 1);//TODO: Why Size()+1?
    std::size_t const numberOfGuys(std::size(guyFrames));
    //int const timelineLength(timeEngine.getTimelineLength());
    //hg::UniverseID const universe(timelineLength);
    assert(numberOfGuys > 0);
    const int guyLineHeight = std::max(static_cast<int>(std::ceil(static_cast<double>(height) / numberOfGuys)), guyLineHeightStandard);

    std::vector<GuyFrameData const*> partitionedFrameData;
    boost::push_back(partitionedFrameData,
        guyFrames
        | boost::adaptors::filtered([](std::optional<GuyFrameData> const &gfd) {return gfd.has_value(); })
        | boost::adaptors::transformed([](std::optional<GuyFrameData> const &gfd) -> auto const* {return &*gfd; })
    );

    boost::sort(partitionedFrameData, [](auto const a, auto const b) { return a->frameNumber < b->frameNumber; });
    auto prevHigh{ boost::begin(partitionedFrameData) };

    for (int frameNumber = 0, end = timelineLength; frameNumber != end; ++frameNumber) {
        //hg::Frame const *const frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        //assert(!isNullFrame(frame));
        int const left = static_cast<int>(frameNumber*timelineContentsWidth / timelineLength);
        //auto const [low, high] = std::equal_range(prevHigh, std::end(partitionedFrameData), frameNumber, [](auto const fn, auto const gfd){return fn < gfd->frameNumber;});
        auto const low{ std::lower_bound(prevHigh, std::end(partitionedFrameData), frameNumber, [](auto const gfd, auto const fn) {return gfd->frameNumber < fn; }) };
        auto const high{ std::upper_bound(low, std::end(partitionedFrameData), frameNumber, [](auto const fn, auto const gfd) {return fn < gfd->frameNumber; }) };
        prevHigh = high;

        for (hg::GuyOutputInfo const &guy :
            boost::make_iterator_range(low, high) | boost::adaptors::transformed([](auto const gfd) -> auto const& {return gfd->guyOutputInfo; }))
        {
            std::size_t const top = static_cast<std::size_t>((height - guyLineHeight)*(guy.getIndex() / static_cast<double>(numberOfGuys)));

            double const xFrac = (guy.getX() - wall.segmentSize()) / static_cast<double>(wall.roomWidth() - 2 * wall.segmentSize());
            double const yFrac = (guy.getY() - wall.segmentSize()) / static_cast<double>(wall.roomHeight() - 2 * wall.segmentSize());

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

void DrawTimeline2(
    sf::RenderTarget &target,
    std::size_t const timelineLength,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID const playerFrame,
    hg::FrameID const timeCursor,
    std::vector<std::optional<GuyFrameData>> const &guyFrames,
    Wall const &wall)
{
    float left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
    float right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
    float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
    float bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::G_TIME_Y + hg::G_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));

    sf::Color const borderColor(100, 100, 100);
    sf::RectangleShape horizontalLine(sf::Vector2f(right - left, 3.f));
    horizontalLine.setFillColor(borderColor);
    horizontalLine.setPosition(left, top - 1.5f);
    target.draw(horizontalLine);
    horizontalLine.setPosition(left, bot - 1.5f);
    target.draw(horizontalLine);

    sf::RectangleShape verticalLine(sf::Vector2f(3.f, bot - top + 3.f));
    verticalLine.setFillColor(borderColor);
    verticalLine.setPosition(left - 3.f, top - 1.5f);
    target.draw(verticalLine);
    verticalLine.setPosition(right, top - 1.5f);
    target.draw(verticalLine);

    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(
        0.f,
        0.f,
        target.getSize().x*static_cast<float>((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X),
        85.f));
    scaledView.setViewport(sf::FloatRect(
        static_cast<float>(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X),
        static_cast<float>(hg::UI_DIVIDE_Y) + static_cast<float>(hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y)),
        1.f - static_cast<float>(hg::UI_DIVIDE_X + 2.*hg::TIMELINE_PAD_X),
        static_cast<float>(hg::G_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))));
    target.setView(scaledView);

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
            cursorTime << (timeCursor.getFrameNumber() * 10 / hg::FRAMERATE) / 10. << "s";
            sf::Text cursorTimeGlyph;
            cursorTimeGlyph.setFont(*hg::defaultFont);
            cursorTimeGlyph.setString(cursorTime.str());
            cursorTimeGlyph.setPosition(timeCursorHorizontalPosition - 3.f, height + 20.f);
            cursorTimeGlyph.setCharacterSize(10);
            cursorTimeGlyph.setFillColor(timeCursorColor);
            cursorTimeGlyph.setOutlineColor(timeCursorColor);
            target.draw(cursorTimeGlyph);
        }
    }
    DrawTimelineContents2(target, height, target.getView().getSize().x, timelineLength, guyFrames, wall);

    target.setView(oldView);
}



void DrawInterfaceBorder(
    sf::RenderTarget &target)
{
    sf::Color const borderColor(0, 0, 0);
    sf::RectangleShape horizontalLine(sf::Vector2f(static_cast<float>(hg::WINDOW_DEFAULT_X), 3.f));
    horizontalLine.setPosition(0.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 1.5f);
    horizontalLine.setFillColor(borderColor);
    target.draw(horizontalLine);

    sf::RectangleShape verticalLine(sf::Vector2f(3.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y)));
    verticalLine.setPosition(static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X) - 1.5f, 0.f);
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
    int const mouseFrame = static_cast<int>((app.getInputState().getMousePosition().x - app.getSize().x*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X))*1. / (app.getSize().x*((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X))*timelineLength);
    return hg::FrameID(clamp(0, mouseFrame, timelineLength-1), hg::UniverseID(timelineLength));
}

std::size_t mousePosToGuyIndex(hg::RenderWindow const &app, hg::TimeEngine const &timeEngine) {
    int const timelineLength = timeEngine.getTimelineLength();
    float timelineOffset = static_cast<float>(app.getSize().x*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
    float timelineWidth = static_cast<float>(app.getSize().x*((1.f - hg::UI_DIVIDE_X) - 2.f*hg::TIMELINE_PAD_X));
    std::size_t const guyFrames = timeEngine.getGuyFrames().size();
    int personalTimelineWidth = (timeEngine.getReplayData().size() > 0) ? std::min(timelineWidth, timelineWidth*static_cast<float>(guyFrames) / static_cast<float>(timeEngine.getTimelineLength())) : timelineWidth;
    
    int mouseGuyIndex = static_cast<int>(static_cast<float>(guyFrames)*(app.getInputState().getMousePosition().x - timelineOffset)*1. / personalTimelineWidth);
    if (mouseGuyIndex < 0)
    {
        mouseGuyIndex = 0;
    }
    else if (mouseGuyIndex > guyFrames - 1)
    {
        mouseGuyIndex = guyFrames - 2;
    }
    return static_cast<std::size_t>(mouseGuyIndex);
}

}
