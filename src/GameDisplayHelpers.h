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
#include "VulkanEngine.h"
#include "GLFWWindow.h"

namespace hg {
    extern sf::Color const uiTextColor;
    void DrawTimeline2(
        sf::RenderTarget &target,
        std::size_t const timelineLength,
        hg::TimeEngine::FrameListList const &waves,
        hg::FrameID const playerFrame,
        hg::FrameID const timeCursor,
        std::vector<std::optional<GuyFrameData>> const &guyFrames,
        Wall const &wall);

    void DrawPersonalTimeline2(
        sf::RenderTarget &target,
        Wall const &wall,
        std::size_t const relativeGuyIndex,
        std::vector<std::optional<GuyFrameData>> const &guyFrames,
        std::vector<GuyInput> const &guyInput,
        std::size_t const minTimelineLength);

    void PlayAudioGlitz(
        hg::mt::std::vector<hg::Glitz> const &glitz,
        AudioPlayingState &audioPlayingState,
        AudioGlitzManager &audioGlitzManager,
        int const guyIndex);
    void DrawVisualGlitzAndWall(
        sf::RenderTarget &target,
        hg::VulkanEngine &eng,
        hg::mt::std::vector<hg::Glitz> const &glitz,
        hg::Wall const &wall,
        hg::LevelResources const &resources,
        sf::Image const &wallImage,
        sf::Image const &positionColoursImage,
        int const guyIndex,
        bool const drawPositionColours);
    void DrawWaves(
        sf::RenderTarget &target,
        hg::TimeEngine::FrameListList const &waves,
        int timelineLength,
        double height);
    void DrawTimelineContents(
        sf::RenderTarget &target,
        hg::TimeEngine const &timeEngine,
        unsigned height);
    void DrawPersonalTimeline(
        sf::RenderTarget &target,
        hg::TimeEngine const &timeEngine,
        std::size_t const relativeGuyIndex,
        std::vector<Frame *> const &guyFrames,
        std::vector<GuyInput> const &guyInput,
        std::size_t const minTimelineLength);
    void DrawColors(sf::RenderTarget &target, sf::Image const &positionColoursImage);
    void DrawTicks(sf::RenderTarget &target, std::size_t const timelineLength);
    void DrawTimeline(
        sf::RenderTarget &target,
        hg::TimeEngine const &timeEngine,
        hg::TimeEngine::FrameListList const &waves,
        hg::FrameID playerFrame,
        hg::FrameID timeCursor,
        int timelineLength);
    void DrawInterfaceBorder(
        sf::RenderTarget &target);
    template<typename BidirectionalGuyRange>
    hg::GuyOutputInfo const &findCurrentGuy(BidirectionalGuyRange const &guyRange, std::size_t index)
    {
        for (GuyOutputInfo const &guyInfo : guyRange)
        {
            if (guyInfo.getIndex() == index)
            {
                return guyInfo;
            }
        }
        assert(false);
        return *boost::begin(guyRange | boost::adaptors::reversed);
    }
    template<typename BidirectionalGuyRange>
    bool const &guyExists(BidirectionalGuyRange const &guyRange, std::size_t index)
    {
        for (GuyOutputInfo const &guyInfo : guyRange)
        {
            if (guyInfo.getIndex() == index)
            {
                return true;
            }
        }
        return false;
    }

    sf::Color guyPositionToColor(double xFrac, double yFrac);
    sf::Color asColor(sf::Vector3<double>const &vec);
    void drawInventory(sf::RenderTarget &app, Pickups const &pickups, hg::Ability abilityCursor);
    hg::mt::std::vector<hg::Glitz> const &getGlitzForDirection(
        hg::FrameView const &view, hg::TimeDirection timeDirection);
    hg::FrameID mousePosToFrameID(GLFWWindow &windowglfw, hg::TimeEngine const &timeEngine);
    std::size_t mousePosToGuyIndex(GLFWWindow &windowglfw, hg::TimeEngine const &timeEngine);
}

#endif //HG_GAME_DISPLAY_HELPERS_H
