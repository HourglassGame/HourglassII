#ifndef HG_GAME_DISPLAY_HELPERS_VULKAN_H
#define HG_GAME_DISPLAY_HELPERS_VULKAN_H

#include "VulkanRenderTarget.h"
#include "hg/GlobalConst.h"
#include "Canvas.h"
#include "hg/Util/Maths.h"
#include "hg/VulkanUtil/VulkanUtil.h"
#include "GameDisplayHelpers.h"
#include "hg/VulkanUtil/VulkanTextureSimple.h"
#include <random>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include "hg/Util/multi_array.h"
#include <boost/range/adaptor/sliced.hpp>
#include <mutex>
#include <locale>
#include <codecvt>
#include <sstream>
#include <gsl/gsl>
#include "VulkanRenderer.h"
#include "hg/Util/memory_util.h"
#include "hg/VulkanUtil/VulkanUpdatableTextureSimple.h"

#include "hg/VulkanUtil/VulkanDescriptorPool.h"
#include "hg/VulkanUtil/VulkanDescriptorSetLayout.h"
#include "hg/VulkanUtil/VulkanUpdatableTextureSimple.h"
#include "hg/VulkanUtil/VulkanTextureSimple.h"

#include <vector>

namespace hg {

inline vec3<float> asColor(vec3<float> const &vec) {
    return {clamp(0.f, vec.a, 1.f), clamp(0.f, vec.b, 1.f), clamp(0.f, vec.c, 1.f)};
}
inline vec3<float> guyPositionToColor(double const xFrac, double const yFrac) {
    vec3<float> const posStart{1.f, 0.f, 0.f};
    vec3<float> const xMax{0.5f, 1.f, 0.f};
    vec3<float> const yMax{0.5f, 0.f, 1.f};

    vec3<float> const xDif{xMax - posStart};
    vec3<float> const yDif{yMax - posStart};

    return asColor(posStart + xDif*gsl::narrow_cast<float>(xFrac) + yDif*gsl::narrow_cast<float>(yFrac));
}

inline std::vector<VkCommandBuffer> createCommandBuffersForRenderer(
    VkDevice const device,
    VkCommandPool const commandPool
)
{
    std::vector<VkCommandBuffer> commandBuffers(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    {
        auto const res{vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data())};
        if (res != VK_SUCCESS) {
            BOOST_THROW_EXCEPTION(std::system_error(res, "failed to allocate command buffers!"));
        }
    }

    return commandBuffers;
}
inline std::vector<VulkanRenderTarget> createRenderTargets(
    VkPhysicalDevice const physicalDevice,
    VkDevice const device,
    VkPipelineLayout const pipelineLayout,
    VkDescriptorSetLayout const projUniformDescriptorSetLayout,
    std::vector<VkCommandBuffer> const &preDrawCommandBuffers,
    std::vector<VkCommandBuffer> const &drawCommandBuffers
) {
    std::vector<VulkanRenderTarget> renderTargets;
    for (auto i{0}; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        renderTargets.emplace_back(
            physicalDevice,
            device,
            pipelineLayout,
            projUniformDescriptorSetLayout,
            preDrawCommandBuffers[i],
            drawCommandBuffers[i]
        );
    }
    return renderTargets;
}

inline VkPipelineLayoutCreateInfo makePipelineLayoutCreateInfo(
    std::array< VkDescriptorSetLayout, 2> const &descriptorSetLayouts
        
    /*VkDescriptorSetLayout const &projUniformDescriptorSetLayout*/)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //TODO: Make 2 set layouts: one for projection uniform and one
    //      for texture.
    pipelineLayoutInfo.setLayoutCount = gsl::narrow<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    /*
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &projUniformDescriptorSetLayout;
    */
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    return pipelineLayoutInfo;
}
inline std::size_t getTimelineTickHeight() {
    return 10;
}

inline VkDescriptorSetLayoutBinding makeUboLayoutBinding(){
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    return uboLayoutBinding;
}
inline VkDescriptorSetLayoutBinding makeSamplerLayoutBinding() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    return samplerLayoutBinding;
}

inline VkDescriptorSetLayoutCreateInfo makeDescriptorSetLayoutCreateInfo(
    VkDescriptorSetLayoutBinding const &descriptorSetLayoutBinding)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &descriptorSetLayoutBinding;
    return layoutInfo;
}
inline vec3<float> interpretAsVulkanColour(unsigned const colour)
{
    return vec3<float>{
        static_cast<float>((colour & 0xFF000000) >> 24)/255.f,
        static_cast<float>((colour & 0x00FF0000) >> 16)/255.f,
        static_cast<float>((colour & 0x0000FF00) >> 8)/ 255.f};
}
inline hg::mt::std::vector<hg::Glitz> const &getGlitzForDirection(
    hg::FrameView const &view, hg::TimeDirection timeDirection)
{
    return timeDirection == TimeDirection::FORWARDS ? view.getForwardsGlitz() : view.getReverseGlitz();
}
inline float scaleHackVal{1.f};

inline void addRectVertices(
    std::vector<Vertex> &vertices,
    float const x,
    float const y,
    float const width,
    float const height,
    vec3<float> const &colour,
    unsigned char const useTexture)
{
    float const left{ x };
    float const right{ x + width };
    float const top{ y };
    float const bottom{ y + height };

    vertices.push_back(
        Vertex{
            vec2<float>{left / scaleHackVal, top / scaleHackVal},
            colour,
            vec2<float>{0.f, 0.f},
            useTexture
        }
    );
    vertices.push_back(
        Vertex{
            vec2<float>{right / scaleHackVal, top / scaleHackVal},
            colour,
            vec2<float>{1.f, 0.f},
            useTexture
        });
    vertices.push_back(
        Vertex{
            vec2<float>{right / scaleHackVal, bottom / scaleHackVal},
            colour,
            vec2<float>{1.f, 1.f},
            useTexture
        });

    vertices.push_back(
        Vertex{
            vec2<float>{right / scaleHackVal, bottom / scaleHackVal},
            colour,
            vec2<float>{1.f, 1.f},
            useTexture
        });
    vertices.push_back(
        Vertex{
            vec2<float>{left / scaleHackVal, bottom / scaleHackVal},
            colour,
            vec2<float>{0.f, 1.f},
            useTexture
        });
    vertices.push_back(
        Vertex{
            vec2<float>{left / scaleHackVal, top / scaleHackVal},
            colour,
            vec2<float>{0.f, 0.f},
            useTexture
        });
}
inline void drawRect(
    VulkanRenderTarget &target,
    float const x,
    float const y,
    float const width,
    float const height,
    vec3<float> const &colour,
    unsigned char const useTexture
)
{
    std::vector<Vertex> vertices;
    addRectVertices(
        vertices, x, y, width, height, colour, useTexture
    );
    target.drawVertices(vertices);
}

template<typename It>
class utf32_from_utf8_iter : public boost::iterator_facade<
    utf32_from_utf8_iter<It>
    , char32_t
    , boost::forward_traversal_tag
    , char32_t>
{
    static std::optional<char32_t> readVal(It &curr, It const &end) {
        if (curr == end) return {};
        using uchar = unsigned char;
        unsigned char const init{gsl::narrow_cast<uchar>(*curr)};

        auto const length{
            (uchar{0b1000'0000} & init) == uchar(0b0000'0000) ? 1 :
            (uchar(0b1110'0000) & init) == uchar(0b1100'0000) ? 2 :
            (uchar(0b1111'0000) & init) == uchar(0b1110'0000) ? 3 :
            /* char32_t{0b1111'1000} & init == char32_t{0b1111'0000} ?*/ 4
        };
        char32_t val{gsl::narrow_cast<char32_t>(
            length == 1 ? (init & uchar(0b0111'1111)) :
            length == 2 ? (init & uchar(0b0001'1111)) :
            length == 3 ? (init & uchar(0b0000'1111)) :
                            (init & uchar(0b0000'0111)))
        };
        int i{0};
        while (true) {
            ++curr;
            ++i;
            if (i == length) return {val};
            if (curr == end) return {};
            val <<= 6;
            val |= (*curr & 0b0011'1111);
        }
    }
    It curr;
    std::optional<char32_t> val;
    It next;
    It end;
public:
    utf32_from_utf8_iter(It curr, It end) :
        curr(curr), val(readVal(curr, end)), next(curr), end(std::move(end))
    {}
    char32_t dereference() const {
        return *val;
    }
    bool equal(utf32_from_utf8_iter const &o) const {
        return curr == o.curr;
    }
    void increment() {
        curr = next;
        val = readVal(next, end);
    }
};


inline void drawText(VulkanRenderTarget &target, VkCommandBuffer const drawCommandBuffer, VkPipelineLayout const pipelineLayout, VkDescriptorSet const fontTexDescriptorSet, std::string const &text, float x, float y, float const size, vec3<float> const colour) {
    auto const vulkanColour{colour};
    float const width{ size / 2 };
    float const height{ size };
    vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &fontTexDescriptorSet, 0, nullptr);

    std::vector<Vertex> vertices;

    auto wasCR_ext{ false };

    float const fontSize{ 16 };
    float const textureRectLeft{ 33 };
    float const textureRectTop{ 64 };
    bool const useTexture{ true };
    for (char32_t const c
        : boost::make_iterator_range(
            utf32_from_utf8_iter(std::begin(text), std::end(text)),
            utf32_from_utf8_iter(std::end(text), std::end(text)))
        ) {
        auto const wasCR{ wasCR_ext };
        wasCR_ext = c == '\r';

        if (c > 0xFFFF) continue;//We only know how to draw the basic multilingual plane for now.

        if (c == '\r' || c == '\n') {
            if (wasCR && c == '\n') {
                continue;
            }
            x = 0;
            y += height * 1.5f;
        }
        else {
            float const left{ x };
            x += width;
            float const right{ x };
            float const top{ y };
            float const bottom{ y + height };

            //TODO: Figure out how texture coordinates work exactly at a pixel level.
            float const texLeft{ textureRectLeft + (c % 0xFF) * fontSize - 0.5f };
            float const texTop{ textureRectTop + (c / 0xFF) * fontSize - 0.5f };
            float const texRight{ texLeft + fontSize / 2 };
            float const texBottom{ texTop + fontSize };

            vertices.push_back(
                Vertex{
                    vec2<float>{left / scaleHackVal, top / scaleHackVal},
                    vulkanColour,
                    vec2<float>{texLeft, texTop},
                    useTexture
                }
            );
            vertices.push_back(
                Vertex{
                    vec2<float>{right / scaleHackVal, top / scaleHackVal},
                    vulkanColour,
                    vec2<float>{texRight, texTop},
                    useTexture
                });
            vertices.push_back(
                Vertex{
                    vec2<float>{right / scaleHackVal, bottom / scaleHackVal},
                    vulkanColour,
                    vec2<float>{texRight, texBottom},
                    useTexture
                });

            vertices.push_back(
                Vertex{
                    vec2<float>{right / scaleHackVal, bottom / scaleHackVal},
                    vulkanColour,
                    vec2<float>{texRight, texBottom},
                    useTexture
                });
            vertices.push_back(
                Vertex{
                    vec2<float>{left / scaleHackVal, bottom / scaleHackVal},
                    vulkanColour,
                    vec2<float>{texLeft, texBottom},
                    useTexture
                });
            vertices.push_back(
                Vertex{
                    vec2<float>{left / scaleHackVal, top / scaleHackVal},
                    vulkanColour,
                    vec2<float>{texLeft, texTop},
                    useTexture
                });
        }
    }

    target.drawVertices(vertices);
}

    
struct GuyFrameData {
    int frameNumber;
    GuyOutputInfo guyOutputInfo;
};
inline VkDescriptorSet createDescriptorSet(
    VkDevice const device,
    VkDescriptorPool const samplerDescriptorPool,
    VkDescriptorSetLayout const textureDescriptorSetLayout,
    VulkanTextureSimple const& texture)
{
    return createDescriptorSet2(device, samplerDescriptorPool, textureDescriptorSetLayout, texture.imageView.imageView, texture.sampler.sampler);
}
inline multi_array<std::optional<VulkanTextureSimple>, 2, 2, 2, 2> loadWallBlockImages(
    VkPhysicalDevice const physicalDevice,
    VkDevice const device,
    VkCommandPool const commandPool,
    VkQueue const graphicsQueue)
{
    multi_array<std::optional<VulkanTextureSimple>, 2, 2, 2, 2> wallBlockImages;
    std::string tilesetName{ "HourglassI" };//TODO: Don't hardcode tileset name!
    for (int right(0); right <= 1; ++right) {
        for (int top(0); top <= 1; ++top) {
            for (int left(0); left <= 1; ++left) {
                for (int bottom(0); bottom <= 1; ++bottom) {
                    std::stringstream filename;
                    filename << "Tilesets/" << tilesetName << right << top << left << bottom << ".png";
                    wallBlockImages[right][top][left][bottom] = std::optional<VulkanTextureSimple>{
                        VulkanTextureSimple{
                            filename.str(),
                            physicalDevice,
                            device,
                            commandPool,
                            graphicsQueue,
                            false
                        }
                    };
                }
            }
        }
    }
    return wallBlockImages;
}

multi_array<VkDescriptorSet, 2, 2, 2, 2> loadWallBlockDescriptorSets(
    VkDevice const device,
    VkDescriptorPool const samplerDescriptorPool,
    VkDescriptorSetLayout const textureDescriptorSetLayout,
    multi_array<std::optional<VulkanTextureSimple>, 2, 2, 2, 2> const& wallBlockImages)
{
    multi_array<VkDescriptorSet, 2, 2, 2, 2> descriptorSets;
    for (int right(0); right <= 1; ++right) {
        for (int top(0); top <= 1; ++top) {
            for (int left(0); left <= 1; ++left) {
                for (int bottom(0); bottom <= 1; ++bottom) {
                    descriptorSets[right][top][left][bottom] =
                        createDescriptorSet(
                            device,
                            samplerDescriptorPool,
                            textureDescriptorSetLayout,
                            *wallBlockImages[right][top][left][bottom]);
                }
            }
        }
    }
    return descriptorSets;
}

inline multi_array<std::optional<VulkanTextureSimple>, 2, 2> loadWallCornerImages(
    VkPhysicalDevice const physicalDevice,
    VkDevice const device,
    VkCommandPool const commandPool,
    VkQueue const graphicsQueue)
{
    multi_array<std::optional<VulkanTextureSimple>, 2, 2> cornerImages;
    std::string tilesetName{ "HourglassI" };//TODO: Don't hardcode tileset name!
    for (int bottom(0); bottom <= 1; ++bottom) {
        for (int right(0); right <= 1; ++right) {
            std::stringstream filename;
            filename << "Tilesets/" << tilesetName << (bottom ? "B" : "T") << (right ? "R" : "L") << ".png";
            cornerImages[bottom][right] = std::optional<VulkanTextureSimple>{
                VulkanTextureSimple{
                    filename.str(),
                    physicalDevice,
                    device,
                    commandPool,
                    graphicsQueue,
                    false
                }
            };
        }
    }
    return cornerImages;
}
multi_array<VkDescriptorSet, 2, 2> loadWallCornerDescriptorSets(
    VkDevice const device,
    VkDescriptorPool const samplerDescriptorPool,
    VkDescriptorSetLayout const textureDescriptorSetLayout,
    multi_array<std::optional<VulkanTextureSimple>, 2, 2> const& wallCornerImages)
{
    multi_array<VkDescriptorSet, 2, 2> descriptorSets;
    for (int bottom(0); bottom <= 1; ++bottom) {
        for (int right(0); right <= 1; ++right) {
            descriptorSets[bottom][right] =
                createDescriptorSet(
                    device,
                    samplerDescriptorPool,
                    textureDescriptorSetLayout,
                    *wallCornerImages[bottom][right]);
        }
    }
    return descriptorSets;
}

inline float getTimelineWidth(VkExtent2D swapChainExtent) {
    return swapChainExtent.width * static_cast<float>((1. - hg::UI_DIVIDE_X) - 2. * hg::TIMELINE_PAD_X);
}
inline std::size_t getTimelineTextureWidth(VkExtent2D swapChainExtent) {
    return static_cast<std::size_t>(std::round(getTimelineWidth(swapChainExtent)));
}
inline std::size_t getTimelineTextureHeight() {
    return 75;
}
inline VulkanDescriptorPool createTimelineTextureDescriptorPool(VkDevice const device) {
    uint32_t const maxSets = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxSets;

    return VulkanDescriptorPool{ device, poolInfo };
}


std::vector<VulkanUpdatableTextureSimple> createTimelineTextures(
    VkPhysicalDevice const physicalDevice,
    VkDevice const device,
    int const width,
    int const height,
    VkDescriptorPool const timelineTextureDescriptorPool,
    VkDescriptorSetLayout const textureDescriptorSetLayout
)
{
    std::vector<VulkanUpdatableTextureSimple> timelineTextures;
    for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        timelineTextures.emplace_back(physicalDevice, device, width, height, timelineTextureDescriptorPool, textureDescriptorSetLayout);
    }
    return timelineTextures;
}
inline VulkanDescriptorPool createSamplerDescriptorPool(VkDevice const device) {
    uint32_t const maxSets =
        13//Glitz Images
        + 2 * 2 * 2 * 2/*Wall Blocks*/
        + 2 * 2 /*Wall Corners*/
        ;//TODO; set this to match count of descriptors used in renderer

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxSets;

    return VulkanDescriptorPool{ device, poolInfo };
#if 0
    std::vector<VulkanDescriptorPool> samplerDescriptorPools;
    for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
        samplerDescriptorPools.emplace_back(
            device,
            poolInfo
        );
    }
    return samplerDescriptorPools;
#endif
}
struct GameDisplayTexDescriptorSets final {
    VkDescriptorSet fontTexDescriptorSet;

    VkDescriptorSet boxTexDescriptorSet;

    VkDescriptorSet boxRTexDescriptorSet;

    VkDescriptorSet balloonTexDescriptorSet;

    VkDescriptorSet balloonRTexDescriptorSet;

    VkDescriptorSet powerupJumpTexDescriptorSet;

    VkDescriptorSet rhinoLeftStopTexDescriptorSet;

    VkDescriptorSet rhinoLeftStopRTexDescriptorSet;

    VkDescriptorSet rhinoRightStopTexDescriptorSet;

    VkDescriptorSet rhinoRightStopRTexDescriptorSet;

    VkDescriptorSet timeGunTexDescriptorSet;

    VkDescriptorSet timeJumpTexDescriptorSet;

    VkDescriptorSet timePauseTexDescriptorSet;

    VkDescriptorSet timeReverseTexDescriptorSet;

    VkDescriptorSet trampolineTexDescriptorSet;
};

#define TEXTURE(NAME, FILE), NAME(FILE, physicalDevice, device, commandPool, graphicsQueue, false) \
, NAME##DescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, NAME))

struct GameDisplayTextures final {
    explicit GameDisplayTextures(
        VkPhysicalDevice const physicalDevice,
        VkDevice const device,
        VkExtent2D const& swapChainExtent,
        VulkanDescriptorSetLayout const& textureDescriptorSetLayout,
        VkCommandPool const commandPool,
        VkQueue const graphicsQueue
    ) :
        samplerDescriptorPool(createSamplerDescriptorPool(device))
        //, samplerDescriptorPools(createSamplerDescriptorPools(device))

        , timelineTextureDescriptorPool(createTimelineTextureDescriptorPool(device))
        , timelineTextures(createTimelineTextures(physicalDevice, device, gsl::narrow<int>(getTimelineTextureWidth(swapChainExtent)), gsl::narrow<int>(getTimelineTextureHeight()), timelineTextureDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout))
        , fontTex("unifont.png", physicalDevice, device, commandPool, graphicsQueue, true)
        , fontTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, fontTex))

        TEXTURE(boxTex, "GlitzData/box.png")
        TEXTURE(boxRTex, "GlitzData/box_r.png")
        TEXTURE(balloonTex, "GlitzData/balloon.png")
        TEXTURE(balloonRTex, "GlitzData/balloon_r.png")
        TEXTURE(powerupJumpTex, "GlitzData/powerup_jump.png")

        TEXTURE(rhinoLeftStopTex, "GlitzData/rhino_left_stop.png")
        TEXTURE(rhinoLeftStopRTex, "GlitzData/rhino_left_stop_r.png")
        TEXTURE(rhinoRightStopTex, "GlitzData/rhino_right_stop.png")
        TEXTURE(rhinoRightStopRTex, "GlitzData/rhino_right_stop_r.png")

        TEXTURE(timeGunTex, "GlitzData/time_gun.png")
        TEXTURE(timeJumpTex, "GlitzData/time_jump.png")
        TEXTURE(timePauseTex, "GlitzData/time_pause.png")
        TEXTURE(timeReverseTex, "GlitzData/time_reverse.png")
        TEXTURE(trampolineTex, "GlitzData/trampoline.png")

        , wallBlockImages(loadWallBlockImages(physicalDevice, device, commandPool, graphicsQueue))
        , wallBlockDescriptorSets(loadWallBlockDescriptorSets(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, wallBlockImages))
        , wallCornerImages(loadWallCornerImages(physicalDevice, device, commandPool, graphicsQueue))
        , wallCornerDescriptorSets(loadWallCornerDescriptorSets(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, wallCornerImages))
        , texDescriptorSets(GameDisplayTexDescriptorSets{
              fontTexDescriptorSet
            , boxTexDescriptorSet
            , boxRTexDescriptorSet
            , balloonTexDescriptorSet
            , balloonRTexDescriptorSet
            , powerupJumpTexDescriptorSet
            , rhinoLeftStopTexDescriptorSet
            , rhinoLeftStopRTexDescriptorSet
            , rhinoRightStopTexDescriptorSet
            , rhinoRightStopRTexDescriptorSet
            , timeGunTexDescriptorSet
            , timeJumpTexDescriptorSet
            , timePauseTexDescriptorSet
            , timeReverseTexDescriptorSet
            , trampolineTexDescriptorSet
            })
    {}
    VulkanDescriptorPool samplerDescriptorPool;
    VulkanDescriptorPool timelineTextureDescriptorPool;

    std::vector<VulkanUpdatableTextureSimple> timelineTextures;

    VulkanTextureSimple fontTex;
    VkDescriptorSet fontTexDescriptorSet;

    VulkanTextureSimple boxTex;
    VkDescriptorSet boxTexDescriptorSet;

    VulkanTextureSimple balloonRTex;
    VkDescriptorSet balloonRTexDescriptorSet;

    VulkanTextureSimple balloonTex;
    VkDescriptorSet balloonTexDescriptorSet;

    VulkanTextureSimple boxRTex;
    VkDescriptorSet boxRTexDescriptorSet;

    VulkanTextureSimple powerupJumpTex;
    VkDescriptorSet powerupJumpTexDescriptorSet;

    VulkanTextureSimple rhinoLeftStopTex;
    VkDescriptorSet rhinoLeftStopTexDescriptorSet;

    VulkanTextureSimple rhinoLeftStopRTex;
    VkDescriptorSet rhinoLeftStopRTexDescriptorSet;

    VulkanTextureSimple rhinoRightStopTex;
    VkDescriptorSet rhinoRightStopTexDescriptorSet;

    VulkanTextureSimple rhinoRightStopRTex;
    VkDescriptorSet rhinoRightStopRTexDescriptorSet;

    VulkanTextureSimple timeGunTex;
    VkDescriptorSet timeGunTexDescriptorSet;

    VulkanTextureSimple timeJumpTex;
    VkDescriptorSet timeJumpTexDescriptorSet;

    VulkanTextureSimple timePauseTex;
    VkDescriptorSet timePauseTexDescriptorSet;

    VulkanTextureSimple timeReverseTex;
    VkDescriptorSet timeReverseTexDescriptorSet;

    VulkanTextureSimple trampolineTex;
    VkDescriptorSet trampolineTexDescriptorSet;

    multi_array<std::optional<VulkanTextureSimple>, 2, 2, 2, 2> wallBlockImages;
    multi_array<VkDescriptorSet, 2, 2, 2, 2> wallBlockDescriptorSets;

    multi_array<std::optional<VulkanTextureSimple>, 2, 2> wallCornerImages;
    multi_array<VkDescriptorSet, 2, 2> wallCornerDescriptorSets;

    GameDisplayTexDescriptorSets texDescriptorSets;
};
class VulkanCanvas2 final : public Canvas
{
public:
    explicit VulkanCanvas2(
        VulkanRenderTarget &rt,
        GameDisplayTexDescriptorSets const &descriptorSets,
        VkCommandBuffer const drawCommandBuffer,
        VkPipelineLayout const pipelineLayout)
        :
        target(&rt), textures(&descriptorSets), drawCommandBuffer(drawCommandBuffer), pipelineLayout(pipelineLayout)
    {}
    void playSound(std::string const &key, int const n) override {
        //soundsToPlay.push_back(AudioGlitzObject(key, n));
    }
    void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
    {
        hg::drawRect(*target, x, y, width, height, interpretAsVulkanColour(colour), 0);
    }
    void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
    {
        vec2<float> const pa{xa, ya};
        vec2<float> const pb{xb, yb};
        if (pa != pb) {
            auto const vulkanColour{interpretAsVulkanColour(colour)};
            std::vector<Vertex> vertices;
            vec2<float> d(normal(pa - pb)*(width / 2.f));
            //Assume VK_FRONT_FACE_CLOCKWISE and VK_CULL_MODE_BACK_BIT
            vec2<float> paNeg{ pa - d };
            vec2<float> pbNeg{ pb - d };
            vec2<float> pbPos{ pb + d };
            vec2<float> paPos{ pa + d };

            vertices.push_back(Vertex{vec2<float>{paNeg.a / scaleHackVal, paNeg.b / scaleHackVal},vulkanColour, vec2<float>{0.f, 0.f}, 0});
            vertices.push_back(Vertex{vec2<float>{pbNeg.a / scaleHackVal, pbNeg.b / scaleHackVal},vulkanColour, vec2<float>{0.f, 0.f}, 0});
            vertices.push_back(Vertex{vec2<float>{pbPos.a / scaleHackVal, pbPos.b / scaleHackVal},vulkanColour, vec2<float>{0.f, 0.f}, 0});

            vertices.push_back(Vertex{vec2<float>{pbPos.a / scaleHackVal, pbPos.b / scaleHackVal},vulkanColour, vec2<float>{0.f, 0.f}, 0});
            vertices.push_back(Vertex{vec2<float>{paPos.a / scaleHackVal, paPos.b / scaleHackVal},vulkanColour, vec2<float>{0.f, 0.f}, 0});
            vertices.push_back(Vertex{vec2<float>{paNeg.a / scaleHackVal, paNeg.b / scaleHackVal},vulkanColour, vec2<float>{0.f, 0.f}, 0});
            target->drawVertices(vertices);
        }
    }
    void drawText(std::string const &text, float x, float const y, float const size, unsigned const colour) override
    {
        hg::drawText(*target, drawCommandBuffer, pipelineLayout, textures->fontTexDescriptorSet, text, x, y, size, interpretAsVulkanColour(colour));
    }
    void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
    {
        if (key == "global.box") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->boxTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.box_r") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->boxRTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.balloon") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->balloonTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.balloon_r") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->balloonRTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.powerup_jump") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->powerupJumpTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.rhino_left_stop") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->rhinoLeftStopTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.rhino_left_stop_r") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->rhinoLeftStopRTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.rhino_right_stop") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->rhinoRightStopTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.rhino_right_stop_r") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->rhinoRightStopRTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.time_gun") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->timeGunTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.time_jump") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->timeJumpTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.time_pause") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->timePauseTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.time_reverse") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->timeReverseTexDescriptorSet, 0, nullptr);
        }
        else if (key == "global.trampoline") {
            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &textures->boxTexDescriptorSet, 0, nullptr);
        }
        hg::drawRect(*target, x, y, width, height, {1.f, 1.f, 1.f}, 1);
    }
    void flushFrame() override {
    }
private:
    VulkanRenderTarget *target;
    GameDisplayTexDescriptorSets const *textures;
    VkCommandBuffer const drawCommandBuffer;
    VkPipelineLayout const pipelineLayout;
};

inline void DrawVisualGlitzAndWall(
            VulkanRenderTarget &target,
            hg::mt::std::vector<hg::Glitz> const &glitz,
            hg::Wall const &wall,
            int const guyIndex,
            VkFramebuffer const targetFrameBuffer,
            VkCommandBuffer const &drawCommandBuffer,
            VkExtent2D const swapChainExtent,
            GameDisplayTexDescriptorSets const &texDescriptorSets,
            VkPipelineLayout const pipelineLayout,
            GameDisplayTextures const &textures)
{
    VkRect2D viewport{};

    {
        //Window Aspect Ratio to Viewport Aspect Ratio conversion rate:
        //Currently the ViewPort is the upper right portion of the window.
        //UI_DIVIDE_X and UI_DIVIDE_Y are the locations of the 2 lines that divide
        //the window into quadrants.
        double const ViewportWidthRatio = 1. - hg::UI_DIVIDE_X;
        double const ViewportHeightRatio = hg::UI_DIVIDE_Y;

        //Viewport Aspect Ratio is
        //VWidth : VHeight
        double const VWidth = swapChainExtent.width*ViewportWidthRatio;
        double const VHeight = swapChainExtent.height*ViewportHeightRatio;

        //Level Aspect Ratio is:
        //LWidth : LHeight
        //Divide by 100. since HG object positions/sizes are fixed point divided by 100
        double const LWidth = wall.roomWidth() / 100.;
        double const LHeight = wall.roomHeight() / 100.;

        if (VWidth == 0 || VHeight == 0 || LWidth == 0 || LHeight == 0) {
            //If the target area has zero size, there is no point drawing anything.
            //If the source area has zero size, the scaling onto the target isn't well defined.
            return;
        }

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

        double const worldViewWidth = LWidth / xFill;
        double const worldViewHeight = LHeight / yFill;

        double const worldViewXPos = -1.*worldViewWidth*((1. - xFill) / 2.);
        double const worldViewYPos = -1.*worldViewHeight*((1. - yFill) / 2.);

        //Have the same aspect ratio as the ViewPort
        assert(essentiallyEqual(worldViewHeight / worldViewWidth, VHeight / VWidth, 0.00001));

        // Exactly fit the level along one axis
        assert(worldViewWidth == LWidth || worldViewHeight == LHeight);
        // Be larger than or equal in size to the level along the other axis,
        assert(worldViewHeight >= LHeight);
        assert(worldViewWidth >= LWidth);
        // With a <= initial position
        assert(worldViewXPos <= 0);
        assert(worldViewYPos <= 0);
        // such that the level is centered in the view
        assert(essentiallyEqual(worldViewXPos + worldViewWidth / 2., LWidth / 2., 0.00001));
        assert(essentiallyEqual(worldViewYPos + worldViewHeight / 2., LHeight / 2., 0.00001));

        //Game view is top-right quadrant

        viewport = VkRect2D {
            VkOffset2D{static_cast<int>(hg::UI_DIVIDE_X*swapChainExtent.width), 0},
            VkExtent2D{
                static_cast<uint32_t>(ViewportWidthRatio*swapChainExtent.width+0.5),
                static_cast<uint32_t>(ViewportHeightRatio*swapChainExtent.height+0.5)
            }
        };

        float const centerX{gsl::narrow_cast<float>(worldViewXPos+worldViewWidth/2.f)};
        float const centerY{gsl::narrow_cast<float>(worldViewYPos+worldViewHeight/2.f)};
        float const sizeX{gsl::narrow_cast<float>(worldViewWidth)};
        float const sizeY{gsl::narrow_cast<float>(worldViewHeight)};
        float const a{2.f / sizeX}; //scale x
        float const b{2.f / sizeY}; //scale y
        float const c{-a * centerX}; //translate x
        float const d{-b * centerY}; //translate y

        target.updateUniformBuffer(
            UniformBufferObject{
            //Out  x    y    z    v
                    a, 0.0, 0.0, 0.0,//In x
                    0.0,   b, 0.0, 0.0,//In y
                    0.0, 0.0, 1.0, 0.0,//In z
                    c,   d, 0.0, 1.0 //In v
            }
        );
        //target.setView(scaledView);
    }
    {
        std::array<VkViewport, 1> viewports{
            {
                static_cast<float>(viewport.offset.x),
                static_cast<float>(viewport.offset.y),
                static_cast<float>(viewport.extent.width),
                static_cast<float>(viewport.extent.height),
                0.0f,
                1.0f
            }
        };
        vkCmdSetViewport(
            drawCommandBuffer,
            0,
            gsl::narrow<uint32_t>(viewports.size()),
            viewports.data()
        );
    }
    hg::VulkanCanvas2 vkCanvas(target, texDescriptorSets, drawCommandBuffer, pipelineLayout);
    hg::LayeredCanvas layeredCanvas(vkCanvas);
    for (hg::Glitz const &particularGlitz : glitz)
    {
        particularGlitz.display(layeredCanvas, guyIndex);
    }
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);

    {

        int const segmentSize = wall.segmentSize() / 100;
        int const roomWidth = wall.roomWidth() / 100;
        int const roomHeight = wall.roomHeight() / 100;

        int const roomIndexWidth = roomWidth / segmentSize;
        int const roomIndexHeight = roomHeight / segmentSize;

        for (int x(0), xend(roomIndexWidth); x != xend; ++x) {
            for (int y(0), yend(roomIndexHeight); y != yend; ++y) {
                if (wall.atIndex(x, y)) {
                    vkCmdBindDescriptorSets(
                        drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1,
                        &textures.wallBlockDescriptorSets
                            [wall.atIndex(x + 1, y)]
                            [wall.atIndex(x, y - 1)]
                            [wall.atIndex(x - 1, y)]
                            [wall.atIndex(x, y + 1)],
                        0, nullptr);

                    drawRect(target, gsl::narrow<float>(x*segmentSize), gsl::narrow<float>(y*segmentSize), gsl::narrow<float>(segmentSize), gsl::narrow<float>(segmentSize), vec3<float>{1.f,1.f,1.f}, 1);

                    for (int vpos(-1); vpos <= 1; vpos += 2) {
                        for (int hpos(-1); hpos <= 1; hpos += 2) {
                            if (wall.atIndex(x + hpos, y)
                                && wall.atIndex(x, y + vpos)
                                && !wall.atIndex(x + hpos, y + vpos))
                            {
                                int const bottom((vpos + 1) / 2);
                                int const right((hpos + 1) / 2);
                                vkCmdBindDescriptorSets(
                                    drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1,
                                    &textures.wallCornerDescriptorSets[bottom][right],
                                    0, nullptr);

                                drawRect(target, gsl::narrow_cast<float>(x*segmentSize + right * segmentSize / 2.), gsl::narrow_cast<float>(y*segmentSize + bottom * segmentSize / 2.), gsl::narrow_cast<float>(segmentSize/2.), gsl::narrow_cast<float>(segmentSize/2.), {1.f,1.f,1.f}, 1);
                            }
                        }
                    }

                }
            }
        }
    }


    flusher.partialFlush(std::numeric_limits<int>::max());
    //if (drawPositionColours) {
    //    DrawColors(target, positionColoursImage);
    //}
    //target.setView(oldView);

    vkCanvas.flushFrame();
    {
        std::array<VkViewport, 1> viewports{
            {
                static_cast<float>(0),
                static_cast<float>(0),
                static_cast<float>(swapChainExtent.width),
                static_cast<float>(swapChainExtent.height),
                0.0f,
                1.0f
            }
        };
        vkCmdSetViewport(
            drawCommandBuffer,
            0,
            gsl::narrow<uint32_t>(viewports.size()),
            viewports.data()
        );
    }
}



inline void DrawPersonalTimeline(
    VulkanRenderTarget &target,
    Wall const &wall,
    VkCommandBuffer const &drawCommandBuffer,
    std::size_t const relativeGuyIndex,
    std::vector<std::vector<GuyFrameData>> const &guyFrames,
    std::vector<GuyInput> const &guyInput,
    std::size_t const minTimelineLength,
    VkExtent2D const swapChainExtent) {

    auto const padding{ 3.f };
    auto const bottomSpace{ 9.f };
    auto const minFrameHeight{ 4.f };
    auto const height{ static_cast<float>(hg::P_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))*hg::WINDOW_DEFAULT_Y - 2.f*padding - bottomSpace };
    //Horizontal Axis:
    // Guy Index
    //Vertical Axis:
    // Frame Index
    {
        float const centerX = WINDOW_DEFAULT_X / 2.f;
        float const centerY = WINDOW_DEFAULT_Y / 2.f;
        float const sizeX = WINDOW_DEFAULT_X;
        float const sizeY = WINDOW_DEFAULT_Y;
        float const a = 2.f / sizeX; //scale x
        float const b = 2.f / sizeY; //scale y
        float const c = -a * centerX; //translate x
        float const d = -b * centerY; //translate y

        target.updateUniformBuffer(
            UniformBufferObject{
                //Out  x    y    z    v
                        a, 0.0, 0.0, 0.0,//In x
                        0.0,   b, 0.0, 0.0,//In y
                        0.0, 0.0, 1.0, 0.0,//In z
                        c,   d, 0.0, 1.0 //In v
            }
        );
    }
    {
        float left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
        float right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
        float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
        float bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::P_TIME_Y + hg::P_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));
        {
            std::vector<Vertex> vertices;

            vec3<float> borderColor{100.f/255.f, 100.f/255.f, 100.f/255.f};

            addRectVertices(vertices, left, top - 1.5f, right - left, 3.f, borderColor, 0);
            addRectVertices(vertices, left, bot - 1.5f, right - left, 3.f, borderColor, 0);

            addRectVertices(vertices, left - 3.f, top - 1.5f, 3.f, bot - top + 3.f, borderColor, 0);
            addRectVertices(vertices, right     , top - 1.5f, 3.f, bot - top + 3.f, borderColor, 0);

            target.drawVertices(vertices);
        }
    }
    float const width = swapChainExtent.width*static_cast<float>((1. - hg::UI_DIVIDE_X) - 2.*hg::TIMELINE_PAD_X);
    {
        float const left = 0.f;
        float const top = 0.f;
        float const width2 = width;
        float const height2 = height + 2.f*padding + bottomSpace;

        float const centerX = left + width2 / 2.f;
        float const centerY = top + height2 / 2.f;
        float const sizeX = width2;
        float const sizeY = height2;
        float const a = 2.f / sizeX; //scale x
        float const b = 2.f / sizeY; //scale y
        float const c = -a * centerX; //translate x
        float const d = -b * centerY; //translate y

        target.updateUniformBuffer(
            UniformBufferObject{
                //Out  x    y    z    v
                        a, 0.0, 0.0, 0.0,//In x
                        0.0,   b, 0.0, 0.0,//In y
                        0.0, 0.0, 1.0, 0.0,//In z
                        c,   d, 0.0, 1.0 //In v
            }
        );
    }
    {
        std::array<VkViewport, 1> viewports{
            {
                static_cast<float>(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X)*swapChainExtent.width,
                (static_cast<float>(hg::UI_DIVIDE_Y) + static_cast<float>(hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)))*swapChainExtent.height,
                static_cast<float>(1.f - static_cast<float>(hg::UI_DIVIDE_X + 2.*hg::TIMELINE_PAD_X))*swapChainExtent.width,
                static_cast<float>(static_cast<float>(hg::P_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y)))*swapChainExtent.height,
                0.0f,
                1.0f
            }
        };
        vkCmdSetViewport(
            drawCommandBuffer,
            0,
            gsl::narrow<uint32_t>(viewports.size()),
            viewports.data()
        );
    }

    std::vector<Vertex> vertices;

    //Colour/Hat = Position/BoxCarrying
    //Waves/Active Frame TODO
    //Time Ticks TODO
    //Special display of dead guy frames? TODO
    std::size_t skipInputFrames = 0;
    //auto const actualGuyFrames{ boost::make_iterator_range(guyFrames.begin(), std::prev(guyFrames.end())) };
    auto const &actualGuyFrames{guyFrames};
    auto const guyFramesLength{ boost::size(guyInput) };
    std::size_t const timelineLength{ std::max(minTimelineLength, guyFramesLength) };

    std::size_t const frameInc = static_cast<std::size_t>(std::max(1, static_cast<int>(std::floor(timelineLength / width))));
    for (std::size_t i{ 0 }; i < guyFramesLength; i += frameInc) {
        auto const frameWidth{ float{width * frameInc / timelineLength } };
        auto const frameHorizontalPosition{ float{i*width / timelineLength} };
        auto const frameHeight{ static_cast<float>(height / static_cast<double>(timelineLength)) };

        if (skipInputFrames > 0)
        {
            --skipInputFrames;
        }
        else if (guyInput[i].getActionTaken())
        {
            vec3<float> colour;
            if (guyInput[i].getPortalUsed())
            {
                colour = vec3<float>{ 50.f/255.f, 255.f / 255.f, 50.f / 255.f };
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else if (guyInput[i].getAbilityUsed())
            {
                colour = vec3<float>{ 50.f / 255.f, 50.f / 255.f, 255.f / 255.f };
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else if (guyInput[i].getBoxAction() || guyInput[i].getBoxLeft() || guyInput[i].getBoxRight())
            {
                colour = vec3<float>{ 0.f / 255.f, 0.f / 255.f, 0.f / 255.f };
                skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
            }
            else
            {
                colour = vec3<float>{ 120.f / 255.f, 120.f / 255.f, 120.f / 255.f };
            }
            //inputLine.setPosition(frameHorizontalPosition, padding + height + frameHeight + minFrameHeight + 1.f);
            //target.draw(inputLine);
            addRectVertices(
                vertices,
                frameHorizontalPosition, padding + height + frameHeight + minFrameHeight + 1.f,
                std::max(frameWidth, 3.f), bottomSpace - minFrameHeight - 1.f + padding / 2.f,
                colour, 0
            );
        }

        //if (isNullFrame(guyFrame)) continue;
        if (actualGuyFrames.size() <= i) {
            break;
        }
        if (actualGuyFrames[i].empty()) {
            continue;
        }

        auto const guyFrame{ actualGuyFrames[i][0] };

        auto const frameVerticalPosition{ float{padding + frameHeight * guyFrame.frameNumber} };

        hg::GuyOutputInfo guy{guyFrame.guyOutputInfo};

        //TODO: Share this logic with DrawTimelineContents!
        double const xFrac = (guy.getX() - wall.segmentSize()) / static_cast<double>(wall.roomWidth() - 2 * wall.segmentSize());
        double const yFrac = (guy.getY() - wall.segmentSize()) / static_cast<double>(wall.roomHeight() - 2 * wall.segmentSize());

        vec3<float> frameColourVulkan{ guyPositionToColor(xFrac, yFrac) };

        addRectVertices(
            vertices,
            frameHorizontalPosition, frameVerticalPosition,
            frameWidth, std::max(minFrameHeight, frameHeight),
            frameColourVulkan, 0
        );

        if (guy.getBoxCarrying() != BoxType::NONE) {
            sf::Color const boxColor(guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                sf::Color(255, 0, 255)
                : sf::Color(0, 255, 0));

            vec3<float> boxColourVulkan{ boxColor.r / 255.f,  boxColor.g / 255.f, boxColor.b / 255.f };

            addRectVertices(
                vertices,
                frameHorizontalPosition, frameVerticalPosition,
                frameWidth, std::max(4.f, frameHeight) / 4.f,
                boxColourVulkan, 0
            );
        }

    }

    addRectVertices(
        vertices,
        (guyFramesLength - relativeGuyIndex)*width / timelineLength, padding,
        3.f, static_cast<float>(height + bottomSpace),
        vec3<float>{200.f/255.f, 200.f / 255.f, 0.f / 255.f}, 0
    );

    target.drawVertices(vertices);
    {
        std::array<VkViewport, 1> viewports{
            {
                static_cast<float>(0),
                static_cast<float>(0),
                static_cast<float>(swapChainExtent.width),
                static_cast<float>(swapChainExtent.height),
                0.0f,
                1.0f
            }
        };
        vkCmdSetViewport(
            drawCommandBuffer,
            0,
            gsl::narrow<uint32_t>(viewports.size()),
            viewports.data()
        );
    }
}



inline void DrawInterfaceBorder(
    VulkanRenderTarget &target,
    VkCommandBuffer const drawCommandBuffer,
    VkExtent2D swapChainExtent)
{
    auto const borderColor{vec3<float>{0.f, 0.f, 0.f}};

    float const centerX = WINDOW_DEFAULT_X / 2.f;
    float const centerY = WINDOW_DEFAULT_Y / 2.f;
    float const sizeX = WINDOW_DEFAULT_X;
    float const sizeY = WINDOW_DEFAULT_Y;
    float const a = 2.f / sizeX; //scale x
    float const b = 2.f / sizeY; //scale y
    float const c = -a * centerX; //translate x
    float const d = -b * centerY; //translate y

    target.updateUniformBuffer(
        UniformBufferObject{
            //Out  x    y    z    v
                    a, 0.0, 0.0, 0.0,//In x
                    0.0,   b, 0.0, 0.0,//In y
                    0.0, 0.0, 1.0, 0.0,//In z
                    c,   d, 0.0, 1.0 //In v
        }
    );

    std::array<VkViewport, 1> viewports{
            {
                0.f,
                0.f,
                static_cast<float>(swapChainExtent.width),
                static_cast<float>(swapChainExtent.height),
                0.0f,
                1.0f
            }
    };
    vkCmdSetViewport(
        drawCommandBuffer,
        0,
        gsl::narrow<uint32_t>(viewports.size()),
        viewports.data()
    );

    drawRect(target, 0.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 1.5f, static_cast<float>(hg::WINDOW_DEFAULT_X), 3.f, borderColor, 0);
    drawRect(target, static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X) - 1.5f, 0.f, 3.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y), borderColor, 0);
}

inline void DrawTimelineContents(
    VulkanRenderTarget &target,
    VulkanUpdatableTextureSimple &timelineTexture,
    VkCommandBuffer const preDrawCommandBuffer,
    VkCommandBuffer const drawCommandBuffer,
    VkPipelineLayout const pipelineLayout,
    //hg::TimeEngine const &timeEngine,
    unsigned const height,
    float const width,
    std::size_t const timelineLength,
    std::vector<std::vector<GuyFrameData>> const &guyFrames,
    Wall const &wall)
{
    static constexpr int boxLineHeight = 1;
    static constexpr int guyLineHeightStandard = 4;
    auto const timelineContentsWidth{ std::round(width) };
    if (timelineContentsWidth == 0) return;
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

    //std::cerr << "start partition" << "\n";
    std::vector<GuyFrameData const*> partitionedFrameData;
    for (unsigned int i = 0; i < guyFrames.size(); ++i)
    {
        for (unsigned int j = 0; j < guyFrames[i].size(); ++j)
        {
            partitionedFrameData.push_back(&guyFrames[i][j]);
            //if (guyFrames[i].size() > 1) {
            //    std::cerr << i << ": " << guyFrames[i][j].frameNumber << "\n";
            //}
        }
    }
    //std::cerr << "end partition" << "\n";

    //boost::push_back(partitionedFrameData,
    //    guyFrames
    //    | boost::adaptors::filtered([](std::optional<GuyFrameData> const &gfd) {return gfd.has_value(); })
    //    | boost::adaptors::transformed([](std::optional<GuyFrameData> const &gfd) -> auto const* {return &*gfd; })
    //);

    boost::sort(partitionedFrameData, [](auto const a, auto const b) { return a->frameNumber < b->frameNumber; });
    auto low{boost::begin(partitionedFrameData)};
    auto const endIt{boost::end(partitionedFrameData)};
    for (int frameNumber{0}, end{gsl::narrow<int>(timelineLength)}; frameNumber != end; ++frameNumber) {
        //hg::Frame const *const frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        //assert(!isNullFrame(frame));
        int const left = static_cast<int>(frameNumber*timelineContentsWidth / timelineLength);
        //auto const [low, high] = std::equal_range(prevHigh, std::end(partitionedFrameData), frameNumber, [](auto const fn, auto const gfd){return fn < gfd->frameNumber;});
        low = std::lower_bound(low, endIt, frameNumber, [](auto const gfd, auto const fn) {return gfd->frameNumber < fn; });
        auto const high{std::upper_bound(low, endIt, frameNumber, [](auto const fn, auto const gfd) {return fn < gfd->frameNumber; }) };
        //prevHigh = high;

        //for (hg::GuyOutputInfo const &guy :
        //    boost::make_iterator_range(low, high) | boost::adaptors::transformed([](auto const gfd) -> auto const& {return gfd->guyOutputInfo; }))
        //for (auto const &gfd : boost::make_iterator_range(low, high))
        for (; low != high; ++low)
        {
            hg::GuyOutputInfo const &guy{(*low)->guyOutputInfo};
            std::size_t const top = static_cast<std::size_t>((height - guyLineHeight)*(guy.getIndex() / static_cast<double>(numberOfGuys)));

            double const xFrac = (guy.getX() - wall.segmentSize()) / static_cast<double>(wall.roomWidth() - 2 * wall.segmentSize());
            double const yFrac = (guy.getY() - wall.segmentSize()) / static_cast<double>(wall.roomHeight() - 2 * wall.segmentSize());

            vec3<float> hgColor{guyPositionToColor(xFrac, yFrac)};
            sf::Color const color(hgColor.a*255,hgColor.b*255,hgColor.c*255);

            std::size_t pos(top);
            for (std::size_t const bot(top + boxLineHeight); pos != bot; ++pos) {
                assert(pos <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
                auto const xPix = static_cast<unsigned int>(left);
                auto const yPix = static_cast<unsigned int>(pos);
                assert(xPix < timelineContents.getSize().x);
                assert(yPix < timelineContents.getSize().y);
                timelineContents.setPixel(
                    xPix, yPix,
                    (guy.getBoxCarrying() == BoxType::NONE) ?
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

    copy_pod_to_storage(
        timelineContents.getPixelsPtr(), timelineContents.getPixelsPtr()+timelineContents.getSize().x*timelineContents.getSize().y * 4,
        timelineTexture.transferSrcMappedRegion.mappedMemory
    );
    timelineTexture.flushTextureChanges(preDrawCommandBuffer);

    vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &timelineTexture.descriptorSet, 0, nullptr);
    hg::drawRect(target, 0.f, gsl::narrow<float>(getTimelineTickHeight()), width, gsl::narrow<float>(height), {1.f,1.f,1.f}, 1);
    //std::cerr << "end Draw Timeline" << "\n";
}
inline void DrawTicks(
    VulkanRenderTarget &target,
    VkCommandBuffer const &drawCommandBuffer,
    std::size_t const timelineLength,
    float const targetWidth)
{
    std::vector<Vertex> vertices;
    vertices.reserve(1+(timelineLength/(5 * FRAMERATE)));
    vec3<float> tickColour{ 0.f, 0.f, 0.f };
    for (std::size_t frameNo(0); frameNo < timelineLength; frameNo += 5 * FRAMERATE) {
        float const left(static_cast<float>(frameNo / static_cast<double>(timelineLength)*targetWidth));
        addRectVertices(vertices, left-1.f, 0.f, 2.f, gsl::narrow<float>(getTimelineTickHeight()), tickColour, 0);
    }
    target.drawVertices(vertices);
}

inline void DrawWaves(
    VulkanRenderTarget &target,
    VkCommandBuffer const drawCommandBuffer,
    hg::TimeEngine::FrameListList const &waves,
    int const timelineLength,
    double const height,
    double const width)
{
    //TODO: This can become slow on HiDPI displays, because it is determined by the width of the display in pixels(?)
    //It also looks bad; due to alisaing artefacts.
    //Come up with a better algorithm.
    auto const waveDisplayWidth{ std::round(width) };
    if (waveDisplayWidth == 0) return;

    std::vector<char> pixelsWhichHaveBeenDrawnIn(static_cast<std::size_t>(waveDisplayWidth));

    std::vector<Vertex> vertices;

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
                    addRectVertices(vertices, gsl::narrow<float>(leftOfWaveRegion), gsl::narrow<float>(getTimelineTickHeight()), gsl::narrow<float>(i - leftOfWaveRegion), static_cast<float>(height), vec3<float>{250.f/255.f, 0.f, 0.f}, 0);

                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            /*
            sf::RectangleShape wavegroup(sf::Vector2f(target.getView().getSize().x - leftOfWaveRegion, static_cast<float>(height)));
            wavegroup.setPosition(static_cast<float>(leftOfWaveRegion), 10.f);
            wavegroup.setFillColor(sf::Color(250, 0, 0));
            target.draw(wavegroup);
            */
            addRectVertices(vertices, gsl::narrow<float>(leftOfWaveRegion), gsl::narrow<float>(getTimelineTickHeight()), gsl::narrow<float>(width - leftOfWaveRegion), static_cast<float>(height), vec3<float>{250.f / 255.f, 0.f, 0.f}, 0);

        }
    }
    target.drawVertices(vertices);

}
inline void DrawTimeline(
    VulkanRenderTarget &target,
    VkCommandBuffer const preDrawCommandBuffer,
    VkCommandBuffer const &drawCommandBuffer,
    VulkanUpdatableTextureSimple &timelineTexture,
    std::size_t const timelineLength,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID const playerFrame,
    hg::FrameID const timeCursor,
    std::vector<std::vector<GuyFrameData>> const &guyFrames,
    Wall const &wall,
    VkExtent2D swapChainExtent,
    VkPipelineLayout pipelineLayout/*
    RunningGameSceneSharedVulkanData * const sceneData,
    hg::TimeEngine const &timeEngine,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID const timeCursor,
    int const timelineLength*/)
{
    {
        float const centerX = WINDOW_DEFAULT_X / 2.f;
        float const centerY = WINDOW_DEFAULT_Y / 2.f;
        float const sizeX = WINDOW_DEFAULT_X;
        float const sizeY = WINDOW_DEFAULT_Y;
        float const a = 2.f / sizeX; //scale x
        float const b = 2.f / sizeY; //scale y
        float const c = -a * centerX; //translate x
        float const d = -b * centerY; //translate y

        target.updateUniformBuffer(
            UniformBufferObject{
                //Out  x    y    z    v
                        a, 0.0, 0.0, 0.0,//In x
                        0.0,   b, 0.0, 0.0,//In y
                        0.0, 0.0, 1.0, 0.0,//In z
                        c,   d, 0.0, 1.0 //In v
            }
        );
    }
    {
        float const left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
        float const right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
        float const top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
        float const bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::G_TIME_Y + hg::G_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));
        {
            std::vector<Vertex> vertices;

            vec3<float> borderColor{ 100.f / 255.f, 100.f / 255.f, 100.f / 255.f };

            addRectVertices(vertices, left, top - 1.5f, right - left, 3.f, borderColor, 0);
            addRectVertices(vertices, left, bot - 1.5f, right - left, 3.f, borderColor, 0);


            addRectVertices(vertices, left - 3.f, top - 1.5f, 3.f, bot - top + 3.f, borderColor, 0);
            addRectVertices(vertices, right, top - 1.5f, 3.f, bot - top + 3.f, borderColor, 0);

            target.drawVertices(vertices);
        }
    }
    float const width{getTimelineWidth(swapChainExtent)};
    {
        float const left{0.f};
        float const top{0.f};
        float const width2{width};
        float const height2{gsl::narrow<float>(getTimelineTextureHeight() + getTimelineTickHeight())};

        float const centerX{left + width2 / 2.f};
        float const centerY{top + height2 / 2.f};
        float const sizeX{width2};
        float const sizeY{height2};
        float const a{2.f / sizeX}; //scale x
        float const b{2.f / sizeY}; //scale y
        float const c{-a * centerX}; //translate x
        float const d{-b * centerY}; //translate y

        target.updateUniformBuffer(
            UniformBufferObject{
                //Out  x    y    z    v
                        a, 0.0, 0.0, 0.0,//In x
                        0.0,   b, 0.0, 0.0,//In y
                        0.0, 0.0, 1.0, 0.0,//In z
                        c,   d, 0.0, 1.0 //In v
            }
        );
    }
    {
        std::array<VkViewport, 1> viewports{
            {

                static_cast<float>(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X)*swapChainExtent.width,
                (static_cast<float>(hg::UI_DIVIDE_Y) + static_cast<float>(hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y)))*swapChainExtent.height,
                (1.f - static_cast<float>(hg::UI_DIVIDE_X + 2.*hg::TIMELINE_PAD_X))*swapChainExtent.width,
                static_cast<float>(hg::G_TIME_HEIGHT*(1. - hg::UI_DIVIDE_Y))*swapChainExtent.height,
                0.0f,
                1.0f
            }
        };
        vkCmdSetViewport(
            drawCommandBuffer,
            0,
            gsl::narrow<uint32_t>(viewports.size()),
            viewports.data()
        );
    }

    auto const height{gsl::narrow<unsigned>(getTimelineTextureHeight())};
    DrawTicks(target, drawCommandBuffer, timelineLength, width);

    DrawWaves(target, drawCommandBuffer, waves, gsl::narrow<int>(timelineLength), height, width);

    if (!isNullFrame(playerFrame)) {
        /*
        sf::RectangleShape playerLine(sf::Vector2f(3.f, static_cast<float>(height)));
        playerLine.setPosition(playerFrame.getFrameNumber()*target.getView().getSize().x / timelineLength, 10.f);
        playerLine.setFillColor(sf::Color(200, 200, 0));
        target.draw(playerLine);
        */
        drawRect(target, gsl::narrow<float>(getFrameNumber(playerFrame)*width / timelineLength), gsl::narrow<float>(getTimelineTickHeight()), 3.f, gsl::narrow<float>(height), vec3<float>{200.f/255.f, 200.f/255.f, 0.f}, 0);
    }

    if (!isNullFrame(timeCursor)) {

        vec3<float> const timeCursorColor{0.f, 0.f, 200.f/255.f};
        float const timeCursorHorizontalPosition{ timeCursor.getFrameNumber()*width / timelineLength };

        drawRect(target, timeCursorHorizontalPosition, gsl::narrow<float>(getTimelineTickHeight()), 3.f, gsl::narrow<float>(height), timeCursorColor, 0);

    }
    DrawTimelineContents(target, timelineTexture, preDrawCommandBuffer, drawCommandBuffer, pipelineLayout, height, width, timelineLength, guyFrames, wall/*, swapChainExtent, sceneData*/);

    {
        std::array<VkViewport, 1> viewports{
            {
                static_cast<float>(0),
                static_cast<float>(0),
                static_cast<float>(swapChainExtent.width),
                static_cast<float>(swapChainExtent.height),
                0.0f,
                1.0f
            }
        };
        vkCmdSetViewport(
            drawCommandBuffer,
            0,
            gsl::narrow<uint32_t>(viewports.size()),
            viewports.data()
        );
    }
}


inline void DrawCrazyTriangle(
    VulkanRenderTarget &target,
    VkCommandBuffer const drawCommandBuffer,
    VkExtent2D swapChainExtent)
{
    target.updateUniformBuffer(
        UniformBufferObject{
            //Out  x    y    z    v
                    1.0, 0.0, 0.0, 0.0,//In x
                    0.0, 1.0, 0.0, 0.0,//In y
                    0.0, 0.0, 1.0, 0.0,//In z
                    0.0, 0.0, 0.0, 1.0 //In v
        }
    );
    std::array viewports{
        VkViewport{
            0.f,
            0.f,
            static_cast<float>(swapChainExtent.width),
            static_cast<float>(swapChainExtent.height),
            0.0f,
            1.0f
        }
    };
    vkCmdSetViewport(
        drawCommandBuffer,
        0,
        gsl::narrow<uint32_t>(viewports.size()),
        viewports.data()
    );

    std::random_device dev;
    std::uniform_real_distribution<float> pos_dist(-1.0f, -0.9f);
    std::uniform_real_distribution<float> col_dist;
    Vertex a{{pos_dist(dev), pos_dist(dev)}, {col_dist(dev),col_dist(dev),col_dist(dev)}, 0};
    Vertex b{{pos_dist(dev), pos_dist(dev)}, {col_dist(dev),col_dist(dev),col_dist(dev)}, 0};
    Vertex c{{pos_dist(dev), pos_dist(dev)}, {col_dist(dev),col_dist(dev),col_dist(dev)}, 0};
    target.drawVertices(
        std::vector<Vertex>{
            a, b, c,
            a, c, b
        }
    );
}


}
#endif // !HG_GAME_DISPLAY_HELPERS_VULKAN_H
