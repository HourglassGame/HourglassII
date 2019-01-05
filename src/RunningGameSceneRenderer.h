#ifndef HG_RUNNING_GAME_SCENE_RENDERER_H
#define HG_RUNNING_GAME_SCENE_RENDERER_H
#include "VulkanRenderTarget.h"
#include "GlobalConst.h"
#include "Canvas.h"
#include "Maths.h"
#include "VulkanUtil.h"
#include "GameDisplayHelpers.h"
#include <random>
#include <boost/range/algorithm/find_if.hpp>
#include <mutex>
namespace hg {
    inline VkDescriptorSetLayoutBinding makeUboLayoutBinding() {
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        return uboLayoutBinding;
    }
    inline VkDescriptorSetLayoutCreateInfo makeDescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutBinding const &uboLayoutBinding) {
        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;
        return layoutInfo;
    }
    sf::Color guyPositionToColor(double xFrac, double yFrac);
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
        vec3<float> const &colour)
    {
        float const left{ x };
        float const right{ x + width };
        float const top{ y };
        float const bottom{ y + height };

        vertices.push_back(
            Vertex{
                vec2<float>{left / scaleHackVal, top / scaleHackVal},
                colour
            }
        );
        vertices.push_back(
            Vertex{
                vec2<float>{right / scaleHackVal, top / scaleHackVal},
                colour
            });
        vertices.push_back(
            Vertex{
                vec2<float>{right / scaleHackVal, bottom / scaleHackVal},
                colour
            });

        vertices.push_back(
            Vertex{
                vec2<float>{right / scaleHackVal, bottom / scaleHackVal},
                colour
            });
        vertices.push_back(
            Vertex{
                vec2<float>{left / scaleHackVal, bottom / scaleHackVal},
                colour
            });
        vertices.push_back(
            Vertex{
                vec2<float>{left / scaleHackVal, top / scaleHackVal},
                colour
            });
    }
    inline void drawRect(
        VulkanRenderTarget &target,
        float const x,
        float const y,
        float const width,
        float const height,
        vec3<float> const &colour
    )
    {
        std::vector<Vertex> vertices;
        addRectVertices(
            vertices, x, y, width, height, colour
        );
        target.drawVertices(vertices);
    }
    class VulkanCanvas2 final : public Canvas
    {
    public:
        explicit VulkanCanvas2(VulkanRenderTarget &rt) :
            target(&rt)
        {}
        void playSound(std::string const &key, int const n) override {
            //soundsToPlay.push_back(AudioGlitzObject(key, n));
        }
        void drawRect(float const x, float const y, float const width, float const height, unsigned const colour) override
        {
            hg::drawRect(*target, x, y, width, height, interpretAsVulkanColour(colour));
        }
        void drawLine(float const xa, float const ya, float const xb, float const yb, float const width, unsigned const colour) override
        {
            sf::Vector2f const pa(xa, ya);
            sf::Vector2f const pb(xb, yb);
            if (pa != pb) {
                auto const vulkanColour{interpretAsVulkanColour(colour)};
                std::vector<Vertex> vertices;
                sf::Vector2f d(normal(pa - pb)*(width / 2.f));
                //Assume VK_FRONT_FACE_CLOCKWISE and VK_CULL_MODE_BACK_BIT
                sf::Vector2f paNeg{ pa - d };
                sf::Vector2f pbNeg{ pb - d };
                sf::Vector2f pbPos{ pb + d };
                sf::Vector2f paPos{ pa + d };

                vertices.push_back(Vertex{vec2<float>{paNeg.x / scaleHackVal, paNeg.y / scaleHackVal},vulkanColour});
                vertices.push_back(Vertex{vec2<float>{pbNeg.x / scaleHackVal, pbNeg.y / scaleHackVal},vulkanColour});
                vertices.push_back(Vertex{vec2<float>{pbPos.x / scaleHackVal, pbPos.y / scaleHackVal},vulkanColour});

                vertices.push_back(Vertex{vec2<float>{pbPos.x / scaleHackVal, pbPos.y / scaleHackVal},vulkanColour});
                vertices.push_back(Vertex{vec2<float>{paPos.x / scaleHackVal, paPos.y / scaleHackVal},vulkanColour});
                vertices.push_back(Vertex{vec2<float>{paNeg.x / scaleHackVal, paNeg.y / scaleHackVal},vulkanColour});
                target->drawVertices(vertices);
            }
        }
        void drawText(std::string const &text, float const x, float const y, float const size, unsigned const colour) override
        {
        }
        void drawImage(std::string const &key, float const x, float const y, float const width, float const height) override
        {
            drawRect(x, y, width, height, (255 << 24 | 0 << 16 | 255 << 8));
        }
        void flushFrame() override {
        }
    private:
        VulkanRenderTarget *target;
    };
    struct GuyFrameData {
        int frameNumber;
        GuyOutputInfo guyOutputInfo;
    };
    struct UIFrameState {
        hg::FrameID drawnFrame;
        hg::TimeDirection drawnTimeDirection;
        std::size_t guyIndex;
        bool shouldDrawGuyPositionColours;
        bool shouldDrawInventory;
        hg::mt::std::map<hg::Ability, int> pickups;
        hg::Ability abilityCursor;
        std::size_t relativeGuyIndex;
        hg::TimeEngine::RunResult waveInfo;
        bool runningFromReplay;
        hg::mt::std::vector<hg::Glitz> drawnGlitz;
        Wall wall;
        std::vector<GuyInput> postOverwriteInput;
        std::size_t timelineLength;
        std::vector<std::optional<GuyFrameData>> guyFrames;
    };

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

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::exception("failed to allocate command buffers!");
        }

        return commandBuffers;
    }
    inline std::vector<VulkanRenderTarget> createRenderTargets(
        VkPhysicalDevice const physicalDevice,
        VkDevice const device,
        VkPipelineLayout const pipelineLayout,
        VkDescriptorSetLayout const descriptorSetLayout,
        std::vector<VkCommandBuffer> const &preDrawCommandBuffers,
        std::vector<VkCommandBuffer> const &drawCommandBuffers
    ) {
        std::vector<VulkanRenderTarget> renderTargets;
        for (auto i{0}; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            renderTargets.emplace_back(
                physicalDevice,
                device,
                pipelineLayout,
                descriptorSetLayout,
                preDrawCommandBuffers[i],
                drawCommandBuffers[i]
            );
        }
        return renderTargets;
    }

    struct RunningGameSceneRenderer {
        ~RunningGameSceneRenderer(){
            //TODO: Use a fence instead, so the device can continue being used elsewhere while RunningGameSceneRenderer is destroyed
            vkDeviceWaitIdle(device);
        }
        RunningGameSceneRenderer(
            VkPhysicalDevice const physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface,
            VkRenderPass const renderPass,
            //VkPipeline const graphicsPipeline,
            //VkPipelineLayout const pipelineLayout,
            //VkDescriptorSetLayout const descriptorSetLayout,
            VkExtent2D const &swapChainExtent)
          : physicalDevice(physicalDevice)
          , device(device)
          , renderPass(renderPass)
          //, graphicsPipeline(graphicsPipeline)
          , swapChainExtent(swapChainExtent)
          , commandPool(device, physicalDevice, surface)
          , preDrawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.commandPool))
          , drawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.commandPool))

          , descriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeUboLayoutBinding()))
          , pipelineLayout(device, swapChainExtent, descriptorSetLayout.descriptorSetLayout)
          , graphicsPipeline(device, swapChainExtent, pipelineLayout.pipelineLayout, renderPass)

          , renderTargets(createRenderTargets(physicalDevice, device, pipelineLayout.pipelineLayout, descriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers))
          , uiFrameState()
          //, timeEngine(nullptr)
        {
        }
        void updateSwapChainData(
            VkRenderPass const renderPass,
            //VkPipeline const graphicsPipeline,
            //VkPipelineLayout const pipelineLayout,
            //VkDescriptorSetLayout const descriptorSetLayout,
            VkExtent2D const &swapChainExtent)
        {
            this->renderPass = renderPass;
            this->swapChainExtent = swapChainExtent;
            renderTargets.clear();
            graphicsPipeline = VulkanGraphicsPipeline(device);
            pipelineLayout = VulkanPipelineLayout(device);

            pipelineLayout = VulkanPipelineLayout(device, swapChainExtent, descriptorSetLayout.descriptorSetLayout);
            graphicsPipeline = VulkanGraphicsPipeline(device, swapChainExtent, pipelineLayout.pipelineLayout, renderPass);
            renderTargets = createRenderTargets(physicalDevice, device, pipelineLayout.pipelineLayout, descriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers);
        }
        std::vector<VkCommandBuffer> renderFrame(
            std::size_t currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
            VkFramebuffer const targetFrameBuffer)
        {
            auto const &preDrawCommandBuffer{ preDrawCommandBuffers[currentFrame] };
            auto const &drawCommandBuffer{ drawCommandBuffers[currentFrame] };
            {
                if (vkResetCommandBuffer(preDrawCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                    throw std::exception("Couldn't reset pre-draw command buffer!");
                }
                if (vkResetCommandBuffer(drawCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                    throw std::exception("Couldn't reset draw command buffer!");
                }

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

                if (vkBeginCommandBuffer(preDrawCommandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::exception("failed to begin recording command buffer!");
                }
                if (vkBeginCommandBuffer(drawCommandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::exception("failed to begin recording command buffer!");
                }

                
                /*
                VkBuffer vertexBufferArr[] = { vertexBuffers[currentFrame].buffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBufferArr, offsets);

                vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
                */
                doRender(preDrawCommandBuffer, drawCommandBuffer, renderTargets[currentFrame], targetFrameBuffer);

                if (vkEndCommandBuffer(drawCommandBuffer) != VK_SUCCESS) {
                    throw std::exception("failed to record command buffer!");
                }
                if (vkEndCommandBuffer(preDrawCommandBuffer) != VK_SUCCESS) {
                    throw std::exception("failed to record command buffer!");
                }
            }
            return { preDrawCommandBuffer, drawCommandBuffer};
        }
        void doRender(VkCommandBuffer const &preDrawCommandBuffer, VkCommandBuffer const &drawCommandBuffer, VulkanRenderTarget &target, VkFramebuffer const targetFrameBuffer){
            target.newFrame();


            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = targetFrameBuffer;
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;// viewport;

            VkClearValue clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(drawCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);

            reallyDoRender(drawCommandBuffer, target, targetFrameBuffer);

            vkCmdEndRenderPass(drawCommandBuffer);
            target.flushBuffersToDevice();

            //vkCmdEndRenderPass(drawCommandBuffer);

            //vkCmdBeginRenderPass(drawCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            //vkCmdEndRenderPass(drawCommandBuffer);
        }
        void setUiFrameState(UIFrameState &&newUiFrameState){
            std::lock_guard<std::mutex> lock{uiFrameStateMutex};
            uiFrameState = std::make_unique<UIFrameState>(std::move(newUiFrameState));
        }
        std::optional<UIFrameState> copyUiFrameState(){
            std::lock_guard<std::mutex> lock{uiFrameStateMutex};
            return uiFrameState ? std::optional<UIFrameState>(*uiFrameState) : std::optional<UIFrameState>{};
        }
        void reallyDoRender(VkCommandBuffer const &drawCommandBuffer, VulkanRenderTarget &target, VkFramebuffer const targetFrameBuffer) {
            
            auto uiFrameStateLocal{copyUiFrameState()};
            if (!uiFrameStateLocal) {return;}
            //target.drawVertices(std::vector<vec2<float>>{ {0.f, -.5f}, { 0.5f,0.5f }, { -0.5f,0.5f }, { -1.f,0.f }, { -1.f,-1.f }, { 0.f,-1.f } });
            DrawVisualGlitzAndWall(
                target,
                uiFrameStateLocal->drawnGlitz/*getGlitzForDirection(timeEngine->getFrame(uiFrameState->drawnFrame)->getView(), uiFrameState->drawnTimeDirection)*/,
                uiFrameStateLocal->wall/*timeEngine->getWall()*/,
                static_cast<int>(uiFrameStateLocal->guyIndex),
                targetFrameBuffer,
                drawCommandBuffer);

#if 0
            DrawTimeline(
                target,
                timeEngine,
                uiFrameState.waveInfo->updatedFrames,
                uiFrameState.drawnFrame,
                timeEngine.getReplayData()[timeEngine.getReplayData().size() - 1].getGuyInput().getTimeCursor(),
                timeEngine.getTimelineLength());
#endif
            DrawPersonalTimeline(
                target,
                uiFrameStateLocal->wall,
                drawCommandBuffer,
                uiFrameStateLocal->relativeGuyIndex,
                //timeEngine->getGuyFrames(),
                uiFrameStateLocal->guyFrames,
                uiFrameStateLocal->postOverwriteInput,//timeEngine->getPostOverwriteInput(),
                uiFrameStateLocal->timelineLength/*static_cast<std::size_t>(timeEngine->getTimelineLength())*/);

            DrawInterfaceBorder(target);
            DrawCrazyTriangle(
                target,
                drawCommandBuffer);
        }

        void DrawCrazyTriangle(
            VulkanRenderTarget &target,
            VkCommandBuffer const &drawCommandBuffer) {
            target.updateUniformBuffer(
                UniformBufferObject{
                    //Out  x    y    z    v
                         1.0, 0.0, 0.0, 0.0,//In x
                         0.0, 1.0, 0.0, 0.0,//In y
                         0.0, 0.0, 1.0, 0.0,//In z
                         0.0, 0.0, 0.0, 1.0 //In v
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
                viewports.size(),
                viewports.data()
            );
            std::random_device dev;
            std::uniform_real_distribution<float> pos_dist(-1.0f, -0.9f);
            std::uniform_real_distribution<float> col_dist;
            Vertex a{{pos_dist(dev), pos_dist(dev)}, {col_dist(dev),col_dist(dev),col_dist(dev)}};
            Vertex b{{pos_dist(dev), pos_dist(dev)}, {col_dist(dev),col_dist(dev),col_dist(dev)}};
            Vertex c{{pos_dist(dev), pos_dist(dev)}, {col_dist(dev),col_dist(dev),col_dist(dev)}};
            target.drawVertices(
                std::vector<Vertex>{
                    a, b, c,
                    a, c, b
                }
            );
        }
        void DrawPersonalTimeline(
            VulkanRenderTarget &target,
            Wall const &wall,
            //hg::TimeEngine const &timeEngine,
            VkCommandBuffer const &drawCommandBuffer,
            std::size_t const relativeGuyIndex,
            //std::vector<Frame *> const &guyFrames,
            std::vector<std::optional<GuyFrameData>> const &guyFrames,
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
            float left = static_cast<float>(hg::WINDOW_DEFAULT_X*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
            float right = static_cast<float>(hg::WINDOW_DEFAULT_X*(1. - hg::TIMELINE_PAD_X));
            float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y)));
            float bot = static_cast<float>(hg::WINDOW_DEFAULT_Y*((hg::UI_DIVIDE_Y) + (hg::P_TIME_Y + hg::P_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y)));
            {
                std::vector<Vertex> vertices;

                vec3<float> borderColor{100.f/255.f, 100.f/255.f, 100.f/255.f};

                addRectVertices(vertices, left, top - 1.5f, right - left, 3.f, borderColor);
                addRectVertices(vertices, left, bot - 1.5f, right - left, 3.f, borderColor);
                /*
                sf::RectangleShape horizontalLine(sf::Vector2f(right - left, 3.f));
                //horizontalLine.setFillColor(borderColor);
                horizontalLine.setPosition(left, top - 1.5f);
                target.draw(horizontalLine);
                horizontalLine.setPosition(left, bot - 1.5f);
                target.draw(horizontalLine);
                */

                addRectVertices(vertices, left - 3.f, top - 1.5f, 3.f, bot - top + 3.f, borderColor);
                addRectVertices(vertices, right     , top - 1.5f, 3.f, bot - top + 3.f, borderColor);
                /*
                sf::RectangleShape verticalLine(sf::Vector2f(3.f, bot - top + 3.f));
                //verticalLine.setFillColor(borderColor);
                verticalLine.setPosition(left - 3.f, top - 1.5f);
                target.draw(verticalLine);
                verticalLine.setPosition(right, top - 1.5f);
                target.draw(verticalLine);
                */
                target.drawVertices(vertices);
            }
            /*
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
            */
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
                    viewports.size(),
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
            auto const guyFramesLength{ boost::size(actualGuyFrames) };
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
                    //sf::RectangleShape inputLine(sf::Vector2f(std::max(frameWidth, 3.f), bottomSpace - minFrameHeight - 1.f + padding / 2.f));
                    vec3<float> colour;
                    if (guyInput[i].getPortalUsed())
                    {
                        //inputLine.setFillColor(sf::Color(50, 255, 50));
                        colour = vec3<float>{ 50.f/255.f, 255.f / 255.f, 50.f / 255.f };
                        skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
                    }
                    else if (guyInput[i].getAbilityUsed())
                    {
                        //inputLine.setFillColor(sf::Color(50, 50, 255));
                        colour = vec3<float>{ 50.f / 255.f, 50.f / 255.f, 255.f / 255.f };
                        skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
                    }
                    else if (guyInput[i].getDown() || guyInput[i].getBoxLeft() || guyInput[i].getBoxRight())
                    {
                        //inputLine.setFillColor(sf::Color(0, 0, 0));
                        colour = vec3<float>{ 0.f / 255.f, 0.f / 255.f, 0.f / 255.f };
                        skipInputFrames = static_cast<std::size_t>(std::floor(2 / frameWidth));
                    }
                    else
                    {
                        //inputLine.setFillColor(sf::Color(120, 120, 120));
                        colour = vec3<float>{ 120.f / 255.f, 120.f / 255.f, 120.f / 255.f };
                    }
                    //inputLine.setPosition(frameHorizontalPosition, padding + height + frameHeight + minFrameHeight + 1.f);
                    //target.draw(inputLine);
                    addRectVertices(
                        vertices,
                        frameHorizontalPosition, padding + height + frameHeight + minFrameHeight + 1.f,
                        std::max(frameWidth, 3.f), bottomSpace - minFrameHeight - 1.f + padding / 2.f,
                        colour
                    );
                }

                auto const guyFrame{ actualGuyFrames[i] };
                //if (isNullFrame(guyFrame)) continue;
                if (!guyFrame) continue;

                auto const frameVerticalPosition{ float{padding + frameHeight * guyFrame->frameNumber} };
                //hg::GuyOutputInfo guy{ *boost::find_if(guyFrame->getView().getGuyInformation(), [i](auto const& guyInfo) {return guyInfo.getIndex() == i; }) };

                hg::GuyOutputInfo guy{guyFrame->guyOutputInfo};

                //TODO: Share this logic with DrawTimelineContents!
                double const xFrac = (guy.getX() - wall.segmentSize()) / static_cast<double>(wall.roomWidth() - 2 * wall.segmentSize());
                double const yFrac = (guy.getY() - wall.segmentSize()) / static_cast<double>(wall.roomHeight() - 2 * wall.segmentSize());

                sf::Color const frameColor(guyPositionToColor(xFrac, yFrac));
                vec3<float> frameColourVulkan{ frameColor.r/255.f,  frameColor.g / 255.f, frameColor.b / 255.f };
                //sf::RectangleShape frameLine(sf::Vector2f(frameWidth, std::max(minFrameHeight, frameHeight)));
                //frameLine.setPosition(frameHorizontalPosition, frameVerticalPosition);
                //frameLine.setFillColor(frameColor);
                //target.draw(frameLine);
                addRectVertices(
                    vertices,
                    frameHorizontalPosition, frameVerticalPosition,
                    frameWidth, std::max(minFrameHeight, frameHeight),
                    frameColourVulkan
                );

                if (guy.getBoxCarrying()) {
                    sf::Color const boxColor(guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                        sf::Color(255, 0, 255)
                        : sf::Color(0, 255, 0));

                    vec3<float> boxColourVulkan{ boxColor.r / 255.f,  boxColor.g / 255.f, boxColor.b / 255.f };
                    /*
                    sf::RectangleShape boxLine(sf::Vector2f(frameWidth, std::max(4.f, frameHeight) / 4.f));
                    boxLine.setPosition(frameHorizontalPosition, frameVerticalPosition);
                    boxLine.setFillColor(boxColor);
                    target.draw(boxLine);
                    */
                    addRectVertices(
                        vertices,
                        frameHorizontalPosition, frameVerticalPosition,
                        frameWidth, std::max(4.f, frameHeight) / 4.f,
                        boxColourVulkan
                    );
                }

            }

            addRectVertices(
                vertices,
                (guyFramesLength - relativeGuyIndex)*width / timelineLength, padding,
                3.f, static_cast<float>(height + bottomSpace),
                vec3<float>{200.f/255.f, 200.f / 255.f, 0.f / 255.f}
            );
            /*
            sf::RectangleShape playerLine(sf::Vector2f(3.f, static_cast<float>(height + bottomSpace)));
            playerLine.setPosition((guyFramesLength - relativeGuyIndex)*target.getView().getSize().x / timelineLength, padding);
            playerLine.setFillColor(sf::Color(200, 200, 0));
            target.draw(playerLine);

            target.setView(oldView);
            */
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
                    viewports.size(),
                    viewports.data()
                );
            }
        }



        void DrawInterfaceBorder(
            VulkanRenderTarget &target)
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
            drawRect(target, 0.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) - 1.5f, static_cast<float>(hg::WINDOW_DEFAULT_X), 3.f, borderColor);
            drawRect(target, static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X) - 1.5f, 0.f, 3.f, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y), borderColor);
        }
        void DrawVisualGlitzAndWall(
            VulkanRenderTarget &target,
            hg::mt::std::vector<hg::Glitz> const &glitz,
            hg::Wall const &wall,
            int const guyIndex,
            VkFramebuffer const targetFrameBuffer,
            VkCommandBuffer const &drawCommandBuffer)
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
                /*
                sf::View scaledView(sf::FloatRect(
                    worldViewXPos,
                    worldViewYPos,
                    worldViewWidth,
                    worldViewHeight));
                */
                //Game view is top-right quadrant
                /*
                scaledView.setViewport(
                    sf::FloatRect(
                        static_cast<float>(hg::UI_DIVIDE_X),
                        0.f,
                        static_cast<float>(ViewportWidthRatio),
                        static_cast<float>(ViewportHeightRatio)));
                */
                viewport = VkRect2D {
                    VkOffset2D{static_cast<int>(hg::UI_DIVIDE_X*swapChainExtent.width), 0},
                    VkExtent2D{
                        static_cast<uint32_t>(ViewportWidthRatio*swapChainExtent.width+0.5),
                        static_cast<uint32_t>(ViewportHeightRatio*swapChainExtent.height+0.5)
                    }
                };

                float const centerX = worldViewXPos+worldViewWidth/2.f;
                float const centerY = worldViewYPos+worldViewHeight/2.f;
                float const sizeX = worldViewWidth;
                float const sizeY = worldViewHeight;
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
                    viewports.size(),
                    viewports.data()
                );
            }
            hg::VulkanCanvas2 vkCanvas(target);
            hg::LayeredCanvas layeredCanvas(vkCanvas);
            for (hg::Glitz const &particularGlitz : glitz)
            {
                particularGlitz.display(layeredCanvas, guyIndex);
            }
            hg::Flusher flusher(layeredCanvas.getFlusher());
            //flusher.partialFlush(1000);

            //sf::Texture wallTex;
            //wallTex.loadFromImage(wallImage);
            //target.draw(sf::Sprite(wallTex));

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
                    viewports.size(),
                    viewports.data()
                );
            }
        }
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkExtent2D swapChainExtent;
        VkRenderPass renderPass;
        //VkPipeline graphicsPipeline;

        VulkanCommandPool commandPool;
        std::vector<VkCommandBuffer> preDrawCommandBuffers;
        std::vector<VkCommandBuffer> drawCommandBuffers;
        VulkanDescriptorSetLayout descriptorSetLayout;
        VulkanPipelineLayout pipelineLayout;
        VulkanGraphicsPipeline graphicsPipeline;
        std::vector<VulkanRenderTarget> renderTargets;
        //TimeEngine const *timeEngine;
        //UIFrameState const *uiFrameState;
        private:
        std::mutex uiFrameStateMutex;
        std::unique_ptr<UIFrameState> uiFrameState;
    };
}
#endif // !HG_RUNNING_GAME_SCENE_RENDERER_H
