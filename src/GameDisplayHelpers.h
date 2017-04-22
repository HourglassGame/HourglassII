#ifndef HG_GAME_DISPLAY_HELPERS_H
#define HG_GAME_DISPLAY_HELPERS_H
#include "RenderWindow.h"
#include "mt/std/vector"
#include "Glitz.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "TimeEngine.h"
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Image.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include "AudioGlitzManager.h"

namespace hg {
    extern sf::Color const uiTextColor;
    void DrawGlitzAndWall(
        hg::RenderWindow &target,
        hg::mt::std::vector<hg::Glitz> const &glitz,
        hg::Wall const &wall,
        hg::LevelResources const &resources,
        AudioPlayingState &audioPlayingState,
        AudioGlitzManager &audioGlitzManager,
        sf::Image const &wallImage);
    void DrawWaves(
        sf::RenderTarget &target,
        hg::TimeEngine::FrameListList const &waves,
        int timelineLength,
        double height);
    void DrawTimelineContents(
        sf::RenderTarget &target,
        hg::TimeEngine const &timeEngine,
        unsigned height);
    void DrawColors(hg::RenderWindow &target, int roomWidth, int roomHeight, int segmentSize);
    void DrawTicks(sf::RenderTarget &target, std::size_t const timelineLength);
    void DrawTimeline(
        sf::RenderTarget &target,
        hg::TimeEngine const &timeEngine,
        hg::TimeEngine::FrameListList const &waves,
        hg::FrameID playerFrame,
        hg::FrameID timeCursor,
        int timelineLength);
    template<typename BidirectionalGuyRange>
    hg::GuyOutputInfo const &findCurrentGuy(BidirectionalGuyRange const &guyRange)
    {
        return *boost::begin(guyRange | boost::adaptors::reversed);
    }

    sf::Color guyPositionToColor(double xFrac, double yFrac);
    sf::Color asColor(sf::Vector3<double>const &vec);
    void drawInventory(hg::RenderWindow &app, hg::mt::std::map<hg::Ability, int> const &pickups, hg::Ability abilityCursor);
    hg::mt::std::vector<hg::Glitz> const &getGlitzForDirection(
        hg::FrameView const &view, hg::TimeDirection timeDirection);
    hg::FrameID mousePosToFrameID(hg::RenderWindow const &app, hg::TimeEngine const &timeEngine);
}

#endif //HG_GAME_DISPLAY_HELPERS_H
