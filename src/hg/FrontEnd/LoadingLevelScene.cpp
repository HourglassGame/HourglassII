#include "LoadingLevelScene.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include "LoadedLevel.h"
#include "hg/Util/async.h"
namespace hg {
    struct LoadingLevelSceneSharedVulkanData {
        explicit LoadingLevelSceneSharedVulkanData(
            PossiblePhysicalDevice const &physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface,
            VkRenderPass const renderPass,
            VkExtent2D const& swapChainExtent,
            VkQueue const graphicsQueue
        )
            : commandPool(device, physicalDevice, surface)
            , preDrawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.h()))
            , drawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.h()))
            , projUniformDescriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeUboLayoutBinding()))
            , textureDescriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeSamplerLayoutBinding()))
            , pipelineLayout(device, makePipelineLayoutCreateInfo({ projUniformDescriptorSetLayout.descriptorSetLayout, textureDescriptorSetLayout.descriptorSetLayout }))
            , graphicsPipeline(device, swapChainExtent, pipelineLayout.pipelineLayout, renderPass)
            , samplerDescriptorPool(createSamplerDescriptorPool(device))
            , renderTargets(createRenderTargets(physicalDevice.physicalDevice, device, pipelineLayout.pipelineLayout, projUniformDescriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers))
            , fontTex("unifont.png", device, physicalDevice.physicalDevice, commandPool.h(), graphicsQueue, true)
            , fontTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, fontTex))
        {}


        VulkanCommandPoolHG commandPool;
        std::vector<VkCommandBuffer> preDrawCommandBuffers;
        std::vector<VkCommandBuffer> drawCommandBuffers;


        VulkanDescriptorSetLayout projUniformDescriptorSetLayout;
        VulkanDescriptorSetLayout textureDescriptorSetLayout;
        VulkanPipelineLayout pipelineLayout;
        VulkanGraphicsPipelineHG graphicsPipeline;
        VulkanDescriptorPool samplerDescriptorPool;
        std::vector<VulkanRenderTarget> renderTargets;
        VulkanTextureSimple fontTex;
        VkDescriptorSet fontTexDescriptorSet;
    };
    struct LoadingLevelSceneFrameVulkanData {

    };

    class LoadingLevelSceneRenderer : public SceneRenderer {
    public:
        explicit LoadingLevelSceneRenderer(
            PossiblePhysicalDevice const &physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface,
            VkRenderPass const renderPass,
            VkExtent2D const& swapChainExtent,
            VkQueue const graphicsQueue
        ) :
              physicalDevice(physicalDevice.physicalDevice)
            , device(device)
            , renderPass(renderPass)
            , swapChainExtent(swapChainExtent)
            , sceneData(
                std::make_shared<LoadingLevelSceneSharedVulkanData>(
                    physicalDevice,
                    device,
                    surface,
                    renderPass,
                    swapChainExtent,
                    graphicsQueue)
            )
            , frameData([] {
            std::vector<std::shared_ptr<LoadingLevelSceneFrameVulkanData>> frameData;
            for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
                frameData.emplace_back(std::make_shared<LoadingLevelSceneFrameVulkanData>());
            }
            return frameData;
                }())
        {}

        VulkanDataKeepAlive getSharedVulkanData() override {
            return VulkanDataKeepAlive{ sceneData };
        }
        VulkanDataKeepAlive getFrameVulkanData(std::size_t const currentFrame) override {
            return VulkanDataKeepAlive{ frameData[currentFrame] };
        }
        void updateSwapChainData(
            VkRenderPass const renderPass,
            VkExtent2D const& swapChainExtent) override
        {
            this->renderPass = renderPass;
            this->swapChainExtent = swapChainExtent;

            sceneData->renderTargets.clear();
            sceneData->graphicsPipeline = VulkanGraphicsPipelineHG(device);


            sceneData->graphicsPipeline = VulkanGraphicsPipelineHG(device, swapChainExtent, sceneData->pipelineLayout.pipelineLayout, renderPass);
            sceneData->renderTargets = createRenderTargets(physicalDevice, device, sceneData->pipelineLayout.pipelineLayout, sceneData->projUniformDescriptorSetLayout.descriptorSetLayout, sceneData->preDrawCommandBuffers, sceneData->drawCommandBuffers);
        }
        std::vector<VkCommandBuffer> renderFrame(
            std::size_t const currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
            VkFramebuffer const targetFrameBuffer) override
        {
            //TODO: Don't duplicate this logic with RunningGameSceneRender::renderFrame
            auto const& preDrawCommandBuffer{ sceneData->preDrawCommandBuffers[currentFrame] };
            auto const& drawCommandBuffer{ sceneData->drawCommandBuffers[currentFrame] };
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
                doRender(preDrawCommandBuffer, drawCommandBuffer, sceneData->renderTargets[currentFrame], targetFrameBuffer);
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
    private:

        void doRender(
            VkCommandBuffer const& preDrawCommandBuffer,
            VkCommandBuffer const& drawCommandBuffer,
            VulkanRenderTarget& target,
            VkFramebuffer const targetFrameBuffer)
        {
            //TODO: don't duplicate this logic with RunningGameSceneRenderer::doRender and MainMenuSceneRenderer and SelectionSceneRenderer
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
            vkCmdBindPipeline(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->graphicsPipeline.h());

            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->pipelineLayout.pipelineLayout, 1, 1, &sceneData->fontTexDescriptorSet, 0, nullptr);

            reallyDoRender(preDrawCommandBuffer, drawCommandBuffer, target, targetFrameBuffer);

            vkCmdEndRenderPass(drawCommandBuffer);
            target.flushBuffersToDevice();
        }

        void reallyDoRender(
            VkCommandBuffer const preDrawCommandBuffer,
            VkCommandBuffer const drawCommandBuffer,
            VulkanRenderTarget& target,
            VkFramebuffer const targetFrameBuffer)
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
                        0.0, b, 0.0, 0.0,//In y
                        0.0, 0.0, 1.0, 0.0,//In z
                        c, d, 0.0, 1.0 //In v
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
            drawText(target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet, "Loading Level...", 520.f, 450.f, 12.f, vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f });
        }

        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkExtent2D swapChainExtent;
        VkRenderPass renderPass;

        std::shared_ptr<LoadingLevelSceneSharedVulkanData> sceneData;
        std::vector<std::shared_ptr<LoadingLevelSceneFrameVulkanData>> frameData;
    };

static std::variant<hg::LoadedLevel, LoadingCanceled_tag>
displayLoadingScreen(
        GLFWWindow &windowglfw,
        boost::future<TimeEngine> &futureLoadedLevel,
        OperationInterrupter &interrupter,
        hg::move_function<LoadedLevel(TimeEngine &&)> const &resourceLoadFun,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer)
{
    LoadingLevelSceneRenderer renderer{
        vulkanEng.physicalDevice,
        vulkanEng.logicalDevice.h(),
        vulkanEng.surface.surface,
        vulkanEng.renderPass.h(),
        vulkanEng.swapChain.extent,
        vulkanEng.logicalDevice.graphicsQ()
    };

    vkRenderer.StartScene(renderer);

    struct RendererCleanupEnforcer final {
        decltype(vkRenderer)& vkRenderer_;
        ~RendererCleanupEnforcer() noexcept {
            vkRenderer_.EndScene();
        }
    } RendererCleanupEnforcer_obj{ vkRenderer };


    bool sceneDrawn = false;
    while (futureLoadedLevel.wait_for(boost::chrono::milliseconds(100)) != boost::future_status::ready) {
        glfwPollEvents();

        {
            if (glfwWindowShouldClose(windowglfw.w)) {
                throw WindowClosed_exception{};
            }

            if (windowglfw.hasLastKey()) {
                int key = windowglfw.useLastKey();
                if (key == GLFW_KEY_ESCAPE) {
                    interrupter.interrupt();
                    futureLoadedLevel.wait();
                    return LoadingCanceled_tag{};
                }
            }
        }
    }
    auto loadedLevel = resourceLoadFun(futureLoadedLevel.get());
    if (interrupter.interrupted()) { return LoadingCanceled_tag{}; }
    return std::move(loadedLevel);
}

std::variant<hg::LoadedLevel, LoadingCanceled_tag>
load_level_scene(
        GLFWWindow &windowglfw,
        LoadLevelFunction const &levelLoadingFunction,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer)
{
    hg::OperationInterrupter interruptor;

    auto futureLoadedLevel = async([&]() {return levelLoadingFunction.timeEngineLoadFun(interruptor); });

    return displayLoadingScreen(windowglfw, futureLoadedLevel, interruptor, levelLoadingFunction.glitzLoadFun, vulkanEng, vkRenderer);
}
}
