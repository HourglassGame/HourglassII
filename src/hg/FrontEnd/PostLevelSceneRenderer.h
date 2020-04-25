#ifndef HG_POST_LEVEL_SCENE_RENDERER_H
#define HG_POST_LEVEL_SCENE_RENDERER_H

namespace hg {
struct PostLevelSceneUiFrameState {
    hg::FrameID drawnFrame;
    bool shouldDrawGuyPositionColours;
    mt::std::vector<hg::Glitz> drawnGlitz;
};

struct PostLevelSceneSharedVulkanData {
    PostLevelSceneSharedVulkanData(
        VkPhysicalDevice const physicalDevice,
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
        , renderTargets(createRenderTargets(physicalDevice, device, pipelineLayout.pipelineLayout, projUniformDescriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers))
        , textures(
            physicalDevice,
            device,
            swapChainExtent,
            textureDescriptorSetLayout,
            commandPool.h(),
            graphicsQueue)
    {
    }

    VulkanCommandPoolHG commandPool;
    std::vector<VkCommandBuffer> preDrawCommandBuffers;
    std::vector<VkCommandBuffer> drawCommandBuffers;

    VulkanDescriptorSetLayout projUniformDescriptorSetLayout;
    VulkanDescriptorSetLayout textureDescriptorSetLayout;
    VulkanPipelineLayout pipelineLayout;
    VulkanGraphicsPipeline graphicsPipeline;
    std::vector<VulkanRenderTarget> renderTargets;

    GameDisplayTextures textures;
};

struct PostLevelSceneFrameVulkanData {
};

class PostLevelSceneRenderer : public SceneRenderer {
public:
    PostLevelSceneRenderer(
        VkPhysicalDevice const physicalDevice,
        VkDevice const device,
        VkSurfaceKHR const surface,
        VkRenderPass const renderPass,
        VkExtent2D const& swapChainExtent,
        VkQueue const graphicsQueue,
        std::vector<std::vector<GuyFrameData>> const &guyFrameData,
        Wall const &wall,
        std::size_t const timelineLength,
        std::vector<GuyInput> const &postOverwriteInput)
        : physicalDevice(physicalDevice)
        , device(device)
        , renderPass(renderPass)
        , swapChainExtent(swapChainExtent)
        
        , sceneData(std::make_shared<PostLevelSceneSharedVulkanData>(
            physicalDevice,
            device,
            surface,
            renderPass,
            swapChainExtent,
            graphicsQueue))
        , frameData([] {
                std::vector<std::shared_ptr<PostLevelSceneFrameVulkanData>> frameData;
                for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
                    frameData.emplace_back(std::make_shared<PostLevelSceneFrameVulkanData>());
                }
                return frameData;
          }())
        , texDescriptorSets({
              sceneData->textures.fontTexDescriptorSet
            , sceneData->textures.boxTexDescriptorSet
            , sceneData->textures.boxRTexDescriptorSet
            , sceneData->textures.powerupJumpTexDescriptorSet
            , sceneData->textures.rhinoLeftStopTexDescriptorSet
            , sceneData->textures.rhinoLeftStopRTexDescriptorSet
            , sceneData->textures.rhinoRightStopTexDescriptorSet
            , sceneData->textures.rhinoRightStopRTexDescriptorSet
            , sceneData->textures.timeGunTexDescriptorSet
            , sceneData->textures.timeJumpTexDescriptorSet
            , sceneData->textures.timePauseTexDescriptorSet
            , sceneData->textures.timeReverseTexDescriptorSet
            , sceneData->textures.trampolineTexDescriptorSet
           })
        , guyFrameData(&guyFrameData)
        , wall(&wall)
        , timelineLength(timelineLength)
        , postOverwriteInput(&postOverwriteInput)
        , uiFrameStateMutex()
        , uiFrameState()
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
        VkExtent2D const& swapChainExtent) override
    {
        this->renderPass = renderPass;
        this->swapChainExtent = swapChainExtent;
        sceneData->renderTargets.clear();
        sceneData->textures.timelineTextures.clear();
        {
            auto const res{ vkResetDescriptorPool(device, sceneData->textures.timelineTextureDescriptorPool.descriptorPool, 0) };
            if (res != VK_SUCCESS) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "timelineTextureDescriptorPool vkResetDescriptorPool failed"));
            }
        }
        sceneData->graphicsPipeline = VulkanGraphicsPipeline(device);

        sceneData->textures.timelineTextures = createTimelineTextures(physicalDevice, device, gsl::narrow<int>(getTimelineTextureWidth(swapChainExtent)), gsl::narrow<int>(getTimelineTextureHeight()), sceneData->textures.timelineTextureDescriptorPool.descriptorPool, sceneData->textureDescriptorSetLayout.descriptorSetLayout);

        sceneData->graphicsPipeline = VulkanGraphicsPipeline(device, swapChainExtent, sceneData->pipelineLayout.pipelineLayout, renderPass);
        sceneData->renderTargets = createRenderTargets(physicalDevice, device, sceneData->pipelineLayout.pipelineLayout, sceneData->projUniformDescriptorSetLayout.descriptorSetLayout, sceneData->preDrawCommandBuffers, sceneData->drawCommandBuffers);
    }
    std::vector<VkCommandBuffer> renderFrame(
        std::size_t currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
        VkFramebuffer const targetFrameBuffer) override
    {
        auto const& preDrawCommandBuffer{ sceneData->preDrawCommandBuffers[currentFrame] };
        auto const& drawCommandBuffer{ sceneData->drawCommandBuffers[currentFrame] };
        //auto const samplerDescriptorPool{samplerDescriptorPools[currentFrame].descriptorPool};
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
            /*
            if (vkResetDescriptorPool(device, samplerDescriptorPool, 0) != VK_SUCCESS) {
                throw std::exception("vkResetDescriptorPool failed");
            }
            */

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

            /*
            VkBuffer vertexBufferArr[] = { vertexBuffers[currentFrame].buffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBufferArr, offsets);

            vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            */
            doRender(preDrawCommandBuffer, drawCommandBuffer, sceneData->renderTargets[currentFrame], targetFrameBuffer, sceneData->textures.timelineTextures[currentFrame]/*, samplerDescriptorPool*/);
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
            VkCommandBuffer const &preDrawCommandBuffer,
            VkCommandBuffer const &drawCommandBuffer,
            VulkanRenderTarget &target,
            VkFramebuffer const targetFrameBuffer,
            VulkanUpdatableTextureSimple &timelineTexture)
        {
            target.newFrame();

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = targetFrameBuffer;
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;// viewport;

            VkClearValue clearColor = { 0.6f, 0.6f, 0.6f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(drawCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->graphicsPipeline.graphicsPipeline);

            vkCmdBindDescriptorSets(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->pipelineLayout.pipelineLayout, 1, 1, &sceneData->textures.fontTexDescriptorSet, 0, nullptr);

            reallyDoRender(preDrawCommandBuffer, drawCommandBuffer, target, targetFrameBuffer, timelineTexture);

            vkCmdEndRenderPass(drawCommandBuffer);
            target.flushBuffersToDevice();
        }
    public:
    void setUiFrameState(PostLevelSceneUiFrameState &&newUiFrameState){
        std::lock_guard<std::mutex> lock{uiFrameStateMutex};
        uiFrameState = std::make_unique<PostLevelSceneUiFrameState>(std::move(newUiFrameState));
    }
    private:
    std::optional<PostLevelSceneUiFrameState> copyUiFrameState(){
        std::lock_guard<std::mutex> lock{uiFrameStateMutex};
        return uiFrameState ? std::optional<PostLevelSceneUiFrameState>(*uiFrameState) : std::optional<PostLevelSceneUiFrameState>{};
    }

    void reallyDoRender(
            VkCommandBuffer const preDrawCommandBuffer,
            VkCommandBuffer const &drawCommandBuffer,
            VulkanRenderTarget &target,
            VkFramebuffer const targetFrameBuffer,
            VulkanUpdatableTextureSimple &timelineTexture)
        {
            auto uiFrameStateLocal{copyUiFrameState()};
            if (!uiFrameStateLocal) {return;}

            DrawVisualGlitzAndWall(
                target,
                uiFrameStateLocal->drawnGlitz,
                *wall,
                -1,//guyIndex
                targetFrameBuffer,
                drawCommandBuffer,
                swapChainExtent,
                texDescriptorSets,
                sceneData->pipelineLayout.pipelineLayout,
                sceneData->textures);

            DrawTimeline(
                target,
                preDrawCommandBuffer,
                drawCommandBuffer,
                timelineTexture,
                timelineLength,
                TimeEngine::FrameListList{},
                uiFrameStateLocal->drawnFrame,
                postOverwriteInput->back().getTimeCursor(),//TODO: Consider not drawing the timeCursor at all.
                *guyFrameData,
                *wall,
                swapChainExtent,
                sceneData->pipelineLayout.pipelineLayout);

            /*
            
            DrawPersonalTimeline(
                target,
                uiFrameStateLocal->wall,
                drawCommandBuffer,
                uiFrameStateLocal->relativeGuyIndex,
                uiFrameStateLocal->guyFrames,
                uiFrameStateLocal->postOverwriteInput,
                uiFrameStateLocal->timelineLength,
                swapChainExtent);

            DrawInterfaceBorder(target, drawCommandBuffer, swapChainExtent);

            DrawCrazyTriangle(
                target,
                drawCommandBuffer);
            
            */

            DrawPersonalTimeline(
                target,
                *wall,
                drawCommandBuffer,
                0/*relativeGuyIndex*/,
                *guyFrameData,
                *postOverwriteInput,
                timelineLength,
                swapChainExtent);

            DrawInterfaceBorder(target, drawCommandBuffer, swapChainExtent);

            DrawCrazyTriangle(target, drawCommandBuffer, swapChainExtent);


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


            {
                std::stringstream currentPlayerIndex;
                currentPlayerIndex << "Index: " << (std::size(*guyFrameData) - 1);

                hg::drawText(
                    target,
                    drawCommandBuffer,
                    sceneData->pipelineLayout.pipelineLayout,
                    texDescriptorSets.fontTexDescriptorSet,
                    currentPlayerIndex.str(),
                    90.f,
                    static_cast<float>(hg::WINDOW_DEFAULT_Y) - 55.f,
                    10.f,
                    UI_TEXT_COLOR);
            }

            
            {
                std::stringstream frameNumberString;
                frameNumberString << "Frame: " << getFrameNumber(uiFrameState->drawnFrame);

                hg::drawText(
                    target,
                    drawCommandBuffer,
                    sceneData->pipelineLayout.pipelineLayout,
                    texDescriptorSets.fontTexDescriptorSet,
                    frameNumberString.str(),
                    90.f,
                    static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) + 60.f,
                    16.f,
                    UI_TEXT_COLOR);
            }
            {
                std::stringstream timeString;
                timeString << "Time: " << (getFrameNumber(uiFrameState->drawnFrame) * 10 / hg::FRAMERATE) / 10. << "s";

                hg::drawText(
                    target,
                    drawCommandBuffer,
                    sceneData->pipelineLayout.pipelineLayout,
                    texDescriptorSets.fontTexDescriptorSet,
                    timeString.str(),
                    90.f,
                    static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) + 20.f,
                    16.f,
                    UI_TEXT_COLOR);
            }
            {
                hg::drawText(
                    target,
                    drawCommandBuffer,
                    sceneData->pipelineLayout.pipelineLayout,
                    texDescriptorSets.fontTexDescriptorSet,
                    "You Won -- Replay",
                    480.f,
                    32.f,
                    16.f,
                    {1.f,0.f,0.f});
            }
            {
                hg::drawText(
                    target,
                    drawCommandBuffer,
                    sceneData->pipelineLayout.pipelineLayout,
                    texDescriptorSets.fontTexDescriptorSet,
                    ", . / keys control the displayed time",
                    380.f,
                    64.f,
                    16.f,
                    {1.f,0.f,0.f});
            }

        }


    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkExtent2D swapChainExtent;
    VkRenderPass renderPass;

    std::shared_ptr<PostLevelSceneSharedVulkanData> sceneData;
    std::vector<std::shared_ptr<PostLevelSceneFrameVulkanData>> frameData;

    GameDisplayTexDescriptorSets texDescriptorSets;
    
    std::vector<std::vector<GuyFrameData>> const *guyFrameData;
    Wall const *wall;
    std::size_t timelineLength;
    std::vector<GuyInput> const *postOverwriteInput;

    std::mutex mutable uiFrameStateMutex;
    std::unique_ptr<PostLevelSceneUiFrameState> uiFrameState;
};


}

#endif // !HG_POST_LEVEL_SCENE_RENDERER_H
