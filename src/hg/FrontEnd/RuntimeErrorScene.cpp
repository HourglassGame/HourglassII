#include "RuntimeErrorScene.h"
#include "Scene.h"
#include "GLFWWindow.h"

namespace hg {



struct RuntimeErrorSceneSharedVulkanData {
    explicit RuntimeErrorSceneSharedVulkanData(
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



class RuntimeErrorSceneRenderer : public SceneRenderer {
public:
    explicit RuntimeErrorSceneRenderer(
        LuaError const& luaError,
        PossiblePhysicalDevice const &physicalDevice,
        VkDevice const device,
        VkSurfaceKHR const surface,
        VkRenderPass const renderPass,
        VkExtent2D const& swapChainExtent,
        VkQueue const graphicsQueue
    ) :
          luaError(&luaError)
        , physicalDevice(physicalDevice.physicalDevice)
        , device(device)
        , renderPass(renderPass)
        , swapChainExtent(swapChainExtent)
        , sceneData(
            std::make_shared<RuntimeErrorSceneSharedVulkanData>(
                physicalDevice,
                device,
                surface,
                renderPass,
                swapChainExtent,
                graphicsQueue)
        )
    {}

    VulkanDataKeepAlive getSharedVulkanData() override {
        return VulkanDataKeepAlive{ sceneData };
    }
    VulkanDataKeepAlive getFrameVulkanData(std::size_t const currentFrame) override {
        return VulkanDataKeepAlive{ std::shared_ptr<void>() };
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

        drawText(target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet, boost::diagnostic_information(*luaError), 0.f, 0.f, 12.f, vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f });
    }

    LuaError const* luaError;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;

    std::shared_ptr<RuntimeErrorSceneSharedVulkanData> sceneData;
};


void report_runtime_error(GLFWWindow &windowglfw, VulkanEngine& vulkanEng, VulkanRenderer& vkRenderer, LuaError const &e)
{
    //TODO: Significantly improve the error reporting (Capture more info, and present it in a more understandable and useful fashion).
    //LuaError can be:
    // illegal call occurred in lua code (Currently this is the only place it is used)
    // lua produced invalid value

    //In each case, we want to report on:
    // What was wrong
    // The back trace (lua and c++)
    //(the c++ backtrace is a 'logical' backtrace, showing which call to
    //lua caused the problem, not the actual c++ call stack)

    //Also include details about how to report on the error so that it can be reproduced.
    // - Log file
    // - Replay File (if error occurred during level running, rather than level loading)
    // - Level Name/basicTriggerSystem/Game Version/etc

    //Print:
    // Lua Backtrace
    // 
    // Precise error
    //


    RuntimeErrorSceneRenderer renderer{
        e,
        vulkanEng.physicalDevice,
        vulkanEng.device.h(),
        vulkanEng.surface.surface,
        vulkanEng.renderPass.h(),
        vulkanEng.swapChain.extent(),
        vulkanEng.device.graphicsQ()
    };
    vkRenderer.StartScene(renderer);

    struct RendererCleanupEnforcer final {
        decltype(vkRenderer)& vkRenderer_;
        ~RendererCleanupEnforcer() noexcept {
            vkRenderer_.EndScene();
        }
    } RendererCleanupEnforcer_obj{ vkRenderer };


    //draw_runtime_error_scene(window, e);

    while (true) {
        glfwPollEvents();
        if (glfwWindowShouldClose(windowglfw.w)) {
            //window.close();
            throw WindowClosed_exception{};
        }
        if (windowglfw.hasLastKey()) {
            if (windowglfw.useLastKey() == GLFW_KEY_ESCAPE) {
                return;
            }
        }
    }
}
}