#include "MainMenuScene.h"
#include "Maths.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include <chrono>
#include <iostream>
#include "VulkanRenderer.h"
namespace hg {
extern sf::Font const *defaultFont;
struct MenuItem final {
    std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag> tag;
    std::string text;
};

struct MainMenuUIFrameState final {
    std::vector<MenuItem> menu;
    int currentItem;
};

struct MainMenuSceneSharedVulkanData {
    MainMenuSceneSharedVulkanData(
        VkPhysicalDevice const physicalDevice,
        VkDevice const device,
        VkSurfaceKHR const surface,
        VkRenderPass const renderPass,
        VkExtent2D const &swapChainExtent,
        VkQueue const graphicsQueue
    )
        : commandPool(device, physicalDevice, surface)
        , preDrawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.commandPool))
        , drawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.commandPool))
        , projUniformDescriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeUboLayoutBinding()))
        , textureDescriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeSamplerLayoutBinding()))
        , pipelineLayout(device, makePipelineLayoutCreateInfo({ projUniformDescriptorSetLayout.descriptorSetLayout, textureDescriptorSetLayout.descriptorSetLayout }))
        , graphicsPipeline(device, swapChainExtent, pipelineLayout.pipelineLayout, renderPass)

        , samplerDescriptorPool(createSamplerDescriptorPool(device))
        //, samplerDescriptorPools(createSamplerDescriptorPools(device))
        , renderTargets(createRenderTargets(physicalDevice, device, pipelineLayout.pipelineLayout, projUniformDescriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers))

        //, timelineTextureDescriptorPool(createTimelineTextureDescriptorPool(device))
        //, timelineTextures(createTimelineTextures(physicalDevice, device, gsl::narrow<int>(getTimelineTextureWidth(swapChainExtent)), gsl::narrow<int>(getTimelineTextureHeight()), timelineTextureDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout))
        , fontTex("unifont.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, true)
        , fontTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, fontTex))
#if 0
        , boxTex("GlitzData/box.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , boxTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, boxTex))

        , boxRTex("GlitzData/box_r.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , boxRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, boxRTex))

        , powerupJumpTex("GlitzData/powerup_jump.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , powerupJumpTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, powerupJumpTex))

        , rhinoLeftStopTex("GlitzData/rhino_left_stop.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , rhinoLeftStopTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoLeftStopTex))

        , rhinoLeftStopRTex("GlitzData/rhino_left_stop_r.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , rhinoLeftStopRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoLeftStopRTex))

        , rhinoRightStopTex("GlitzData/rhino_right_stop.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , rhinoRightStopTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoRightStopTex))

        , rhinoRightStopRTex("GlitzData/rhino_right_stop_r.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , rhinoRightStopRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoRightStopRTex))

        , timeGunTex("GlitzData/time_gun.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , timeGunTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timeGunTex))

        , timeJumpTex("GlitzData/time_jump.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , timeJumpTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timeJumpTex))

        , timePauseTex("GlitzData/time_pause.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , timePauseTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timePauseTex))

        , timeReverseTex("GlitzData/time_reverse.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , timeReverseTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timeReverseTex))

        , trampolineTex("GlitzData/trampoline.png", device, physicalDevice, commandPool.commandPool, graphicsQueue, false)
        , trampolineTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, trampolineTex))

        , wallBlockImages(loadWallBlockImages(device, physicalDevice, commandPool.commandPool, graphicsQueue))
        , wallBlockDescriptorSets(loadWallBlockDescriptorSets(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, wallBlockImages))
        , wallCornerImages(loadWallCornerImages(device, physicalDevice, commandPool.commandPool, graphicsQueue))
        , wallCornerDescriptorSets(loadWallCornerDescriptorSets(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, wallCornerImages))
        , texDescriptorSets(TexDescriptorSets{
                  fontTexDescriptorSet
                , boxTexDescriptorSet
                , boxRTexDescriptorSet
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
#endif
    {
    }

    VulkanCommandPool commandPool;
    std::vector<VkCommandBuffer> preDrawCommandBuffers;
    std::vector<VkCommandBuffer> drawCommandBuffers;

    VulkanDescriptorSetLayout projUniformDescriptorSetLayout;
    VulkanDescriptorSetLayout textureDescriptorSetLayout;
    VulkanPipelineLayout pipelineLayout;
    VulkanGraphicsPipeline graphicsPipeline;
    VulkanDescriptorPool samplerDescriptorPool;
    //std::vector<VulkanDescriptorPool> samplerDescriptorPools;
    std::vector<VulkanRenderTarget> renderTargets;
    //VulkanDescriptorPool timelineTextureDescriptorPool;

    //std::vector<VulkanUpdatableTextureSimple> timelineTextures;
    //std::vector<>
    VulkanTextureSimple fontTex;
    VkDescriptorSet fontTexDescriptorSet;
#if 0
    VulkanTextureSimple boxTex;
    VkDescriptorSet boxTexDescriptorSet;

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
    TexDescriptorSets texDescriptorSets;
#endif
};
struct MainMenuSceneFrameVulkanData {

};
class MainMenuSceneRenderer : public SceneRenderer {
public:
    MainMenuSceneRenderer(
        VkPhysicalDevice const physicalDevice,
        VkDevice const device,
        VkSurfaceKHR const surface,
        VkRenderPass const renderPass,
        VkExtent2D const &swapChainExtent,
        VkQueue const graphicsQueue)
        : physicalDevice(physicalDevice)
        , device(device)
        , renderPass(renderPass)
        , swapChainExtent(swapChainExtent)
        , sceneData(std::make_shared<MainMenuSceneSharedVulkanData>(
            physicalDevice,
            device,
            surface,
            renderPass,
            swapChainExtent,
            graphicsQueue))
        , frameData([] {
            std::vector<std::shared_ptr<MainMenuSceneFrameVulkanData>> frameData;
            for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
                frameData.emplace_back(std::make_shared<MainMenuSceneFrameVulkanData>());
            }
            return frameData;
          }())
        , mainMenuUiFrameState()
        //, timeEngine(nullptr)
    {
    }

    VulkanDataKeepAlive getSharedVulkanData() override {
        return VulkanDataKeepAlive{ sceneData };
    }
    VulkanDataKeepAlive getFrameVulkanData(std::size_t const currentFrame) override {
        return VulkanDataKeepAlive{ frameData[currentFrame] };
    }
    void updateSwapChainData(
        VkRenderPass const renderPass,
        VkExtent2D const &swapChainExtent) override
    {
        this->renderPass = renderPass;
        this->swapChainExtent = swapChainExtent;
        sceneData->renderTargets.clear();
        sceneData->graphicsPipeline = VulkanGraphicsPipeline(device);
        //pipelineLayout = VulkanPipelineLayout(device);

        //pipelineLayout = VulkanPipelineLayout(device, makePipelineLayoutCreateInfo(descriptorSetLayout.descriptorSetLayout));
        sceneData->graphicsPipeline = VulkanGraphicsPipeline(device, swapChainExtent, sceneData->pipelineLayout.pipelineLayout, renderPass);
        sceneData->renderTargets = createRenderTargets(physicalDevice, device, sceneData->pipelineLayout.pipelineLayout, sceneData->projUniformDescriptorSetLayout.descriptorSetLayout, sceneData->preDrawCommandBuffers, sceneData->drawCommandBuffers);
    }
    std::vector<VkCommandBuffer> renderFrame(
        std::size_t const currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
        VkFramebuffer const targetFrameBuffer) override
    {
        //TODO: Don't duplicate this logic with RunningGameSceneRender::renderFrame
        auto const &preDrawCommandBuffer{ sceneData->preDrawCommandBuffers[currentFrame] };
        auto const &drawCommandBuffer{ sceneData->drawCommandBuffers[currentFrame] };
        {
            {
                auto const res{ vkResetCommandBuffer(preDrawCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't reset pre-draw command buffer!"));
                }
            }
            {
                auto const res{ vkResetCommandBuffer(drawCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't reset draw command buffer!"));
                }
            }

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            {
                auto const res{ vkBeginCommandBuffer(preDrawCommandBuffer, &beginInfo) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to begin recording preDraw command buffer!"));
                }
            }
            {
                auto const res{ vkBeginCommandBuffer(drawCommandBuffer, &beginInfo) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to begin recording draw command buffer!"));
                }
            }
            doRender(preDrawCommandBuffer, drawCommandBuffer, sceneData->renderTargets[currentFrame], targetFrameBuffer/*, sceneData->timelineTextures[currentFrame]*//*, samplerDescriptorPool*/);
            {
                auto const res{ vkEndCommandBuffer(drawCommandBuffer) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to record draw command buffer!"));
                }
            }
            {
                auto const res{ vkEndCommandBuffer(preDrawCommandBuffer) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to record preDraw command buffer!"));
                }
            }
        }
        return { preDrawCommandBuffer, drawCommandBuffer };
    }
    
    void setUiFrameState(MainMenuUIFrameState &&newMainMenuUiFrameState) {
        std::lock_guard<std::mutex> lock{ mainMenuUiFrameStateMutex };
        mainMenuUiFrameState = std::make_unique<MainMenuUIFrameState>(std::move(newMainMenuUiFrameState));
    }
    ~MainMenuSceneRenderer() {
        //TODO: Use a fence instead, so the device can continue being used elsewhere while RunningGameSceneRenderer is destroyed
        //This should be managed by VulkanRenderer
        //vkDeviceWaitIdle(device);
    }
private:
    void doRender(
        VkCommandBuffer const &preDrawCommandBuffer,
        VkCommandBuffer const &drawCommandBuffer,
        VulkanRenderTarget &target,
        VkFramebuffer const targetFrameBuffer)
    {
        //TODO: don't duplicate this logic with RunningGameSceneRenderer::doRender
        target.newFrame();

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = targetFrameBuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;// viewport;

        VkClearValue clearColor = { 0.5f, 0.5f, 0.5f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(drawCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->graphicsPipeline.graphicsPipeline);

        vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->pipelineLayout.pipelineLayout, 1, 1, &sceneData->fontTexDescriptorSet, 0, nullptr);

        reallyDoRender(preDrawCommandBuffer, drawCommandBuffer, target, targetFrameBuffer);

        vkCmdEndRenderPass(drawCommandBuffer);
        target.flushBuffersToDevice();
    }

    void reallyDoRender(
        VkCommandBuffer const preDrawCommandBuffer,
        VkCommandBuffer const drawCommandBuffer,
        VulkanRenderTarget &target,
        VkFramebuffer const targetFrameBuffer)
    {
        auto uiFrameStateLocal{ copyUiFrameState() };
        if (!uiFrameStateLocal) { return; }
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
        }

        float ypos = 200.f;
        int i = 0;
        for (auto const& menuItem : uiFrameStateLocal->menu) {
#if 0
            sf::Text menuItemGlyph;
            menuItemGlyph.setFont(*hg::defaultFont);
            menuItemGlyph.setString(menuItem.text);
            auto const menuItemColour = i == uiFrameStateLocal->currentItem ? sf::Color(0, 255, 255) : sf::Color(255, 255, 255);
            menuItemGlyph.setFillColor(menuItemColour);
            menuItemGlyph.setOutlineColor(menuItemColour);
            menuItemGlyph.setPosition(450.f, ypos);
            menuItemGlyph.setCharacterSize(42);
            window.draw(menuItemGlyph);
#endif
            drawText(target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet, menuItem.text, 450.f, ypos, 42.f, i == uiFrameStateLocal->currentItem ? vec3<float>{0.f/255.f, 255.f / 255.f, 255.f / 255.f} : vec3<float>{255.f / 255.f, 255.f / 255.f, 255.f / 255.f});

            ypos += 50.f;
            ++i;
        }

    }
    std::optional<MainMenuUIFrameState> copyUiFrameState() {
        std::lock_guard<std::mutex> lock{ mainMenuUiFrameStateMutex };
        return mainMenuUiFrameState ? std::optional<MainMenuUIFrameState>(*mainMenuUiFrameState) : std::optional<MainMenuUIFrameState>{};
    }

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;

    std::shared_ptr<MainMenuSceneSharedVulkanData> sceneData;
    std::vector<std::shared_ptr<MainMenuSceneFrameVulkanData>> frameData;

    //TexDescriptorSets texDescriptorSets;

    mutable std::mutex mainMenuUiFrameStateMutex;
    std::unique_ptr<MainMenuUIFrameState> mainMenuUiFrameState;
};

static void drawMainMenu(hg::RenderWindow &window, std::vector<MenuItem> const &menu, int const currentItem) {
    window.clear();
    float ypos = 200.f;
    int i = 0;
    for (auto const& menuItem: menu) {
        sf::Text menuItemGlyph;
        menuItemGlyph.setFont(*hg::defaultFont);
        menuItemGlyph.setString(menuItem.text);
        auto const menuItemColour = i == currentItem ? sf::Color(0, 255, 255) : sf::Color(255, 255, 255);
        menuItemGlyph.setFillColor(menuItemColour);
        menuItemGlyph.setOutlineColor(menuItemColour);
        menuItemGlyph.setPosition(450.f, ypos);
        ypos += 50.f;
        menuItemGlyph.setCharacterSize(42);
        window.draw(menuItemGlyph);

        ++i;
    }
    window.display();
}

std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag>
run_main_menu(
    hg::RenderWindow &window, 
    GLFWWindow &windowglfw, 
    VulkanEngine &vulkanEng, 
    VulkanRenderer &vkRenderer)
{
    int currentItem = 0;
    std::vector<MenuItem> const menu {
        {RunALevel_tag{}, "Run Level"},
        {RunAReplay_tag{}, "Run Replay"},
        {Exit_tag{}, "Exit"}
    };
    MainMenuSceneRenderer renderer{
        vulkanEng.physicalDevice,
        vulkanEng.logicalDevice.device,
        vulkanEng.surface.surface,
        vulkanEng.renderPass.renderPass,
        //eng.graphicsPipeline.graphicsPipeline,
        //eng.pipelineLayout.pipelineLayout,
        //eng.descriptorSetLayout.descriptorSetLayout,
        vulkanEng.swapChain.extent,
        vulkanEng.logicalDevice.graphicsQueue
    };
    vkRenderer.StartScene(renderer);

    struct RendererCleanupEnforcer final {
        decltype(vkRenderer) &vkRenderer_;
        ~RendererCleanupEnforcer() noexcept {
            vkRenderer_.EndScene();
        }
    } RendererCleanupEnforcer_obj{ vkRenderer };

    auto frameStart = std::chrono::steady_clock::now();
    while (true) {
        renderer.setUiFrameState(MainMenuUIFrameState{ menu, currentItem });
        drawMainMenu(window, menu, currentItem);

        bool mainMenuDrawn = true;
        while (mainMenuDrawn) {
            glfwPollEvents();

            //vulkanEng.drawFrame();
            auto nextFrame = std::chrono::steady_clock::now();
            std::chrono::duration<double> frameTime = nextFrame - frameStart;
            //std::cout << std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count() << "\n";
            frameStart = nextFrame;

            if (glfwWindowShouldClose(windowglfw.w)) {
                std::cout << "Main Menu windowglfw Pointer: " << &windowglfw << "\n" << std::flush;
                std::cout << "Main Menu windowglfw.w Pointer: " << (windowglfw.w) << "\n" << std::flush;
                window.close();
                throw WindowClosed_exception{};
            }

            if (windowglfw.hasLastKey()) {
                int key = windowglfw.useLastKey();
                if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
                    return menu[currentItem].tag;
                }
                if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
                    currentItem = flooredModulo(currentItem - 1, static_cast<int>(menu.size()));
                    mainMenuDrawn = false;
                }
                if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
                    currentItem = flooredModulo(currentItem + 1, static_cast<int>(menu.size()));
                    mainMenuDrawn = false;
                }
            }
        }
    }
}

}
