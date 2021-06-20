#ifndef HG_RUNNING_GAME_SCENE_RENDERER_H
#define HG_RUNNING_GAME_SCENE_RENDERER_H
#include "VulkanRenderTarget.h"
#include "hg/GlobalConst.h"
#include "Hg_Input.h"
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
#include <cmath>
#include <gsl/gsl>
#include "VulkanRenderer.h"
#include "hg/Util/memory_util.h"
#include "GameDisplayHelpersVulkan.h"
#include "hg/VulkanUtil/VulkanUpdatableTextureSimple.h"

namespace hg {
	struct UIFrameState {
		hg::FrameID drawnFrame;
		hg::TimeDirection drawnTimeDirection;
		std::size_t guyIndex;
		bool shouldDrawGuyPositionColours;
		bool shouldDrawInventory;
		Pickups pickups;
		hg::Input input;
		bool paused;
		std::size_t relativeGuyIndex;
		hg::TimeEngine::RunResult waveInfo;
		bool runningFromReplay;
		bool guyFrameUpdated;
		mt::std::vector<hg::Glitz> drawnGlitz;
		Wall wall;
		std::vector<GuyInput> postOverwriteInput;
		std::size_t timelineLength;
		std::vector<std::vector<GuyFrameData>> guyFrames;
	};

	struct RunningGameSceneSharedVulkanData {
		RunningGameSceneSharedVulkanData(
			PossiblePhysicalDevice const &physicalDevice,
			VkDevice const device,
			VkSurfaceKHR const surface,
			VkRenderPass const renderPass,
			VkExtent2D const &swapChainExtent,
			VkQueue const graphicsQueue
		)
			: commandPool(device, physicalDevice, surface)
			, preDrawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.h()))
			, drawCommandBuffers(createCommandBuffersForRenderer(device, commandPool.h()))
			, projUniformDescriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeUboLayoutBinding()))
			, textureDescriptorSetLayout(device, makeDescriptorSetLayoutCreateInfo(makeSamplerLayoutBinding()))
			, pipelineLayout(device, makePipelineLayoutCreateInfo({ projUniformDescriptorSetLayout.descriptorSetLayout, textureDescriptorSetLayout.descriptorSetLayout }))
			, graphicsPipeline(device, swapChainExtent, pipelineLayout.pipelineLayout, renderPass)
			, renderTargets(createRenderTargets(physicalDevice.physicalDevice, device, pipelineLayout.pipelineLayout, projUniformDescriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers))
			, textures(
				physicalDevice.physicalDevice,
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
		VulkanGraphicsPipelineHG graphicsPipeline;
		std::vector<VulkanRenderTarget> renderTargets;

		GameDisplayTextures textures;
	};

	struct RunningGameSceneFrameVulkanData {
	};


	class RunningGameSceneRenderer : public SceneRenderer {
	public:
		~RunningGameSceneRenderer(){
			//TODO: Use a fence instead, so the device can continue being used elsewhere while RunningGameSceneRenderer is destroyed
			//vkDeviceWaitIdle(device);
		}
		RunningGameSceneRenderer(
			PossiblePhysicalDevice const &physicalDevice,
			VkDevice const device,
			VkSurfaceKHR const surface,
			VkRenderPass const renderPass,
			VkExtent2D const &swapChainExtent,
			VkQueue const graphicsQueue)
		  : physicalDevice(physicalDevice.physicalDevice)
		  , device(device)
		  , renderPass(renderPass)
		  , swapChainExtent(swapChainExtent)
		  , sceneData(std::make_shared<RunningGameSceneSharedVulkanData>(
			  physicalDevice,
			  device,
			  surface,
			  renderPass,
			  swapChainExtent,
			  graphicsQueue))
		  , frameData([]{
				std::vector<std::shared_ptr<RunningGameSceneFrameVulkanData>> frameData;
				for (auto i{0}; i != MAX_FRAMES_IN_FLIGHT; ++i) {
					frameData.emplace_back(std::make_shared<RunningGameSceneFrameVulkanData>());
				}
				return frameData;
			}())
		  , texDescriptorSets(GameDisplayTexDescriptorSets{
				sceneData->textures.fontTexDescriptorSet
			  , sceneData->textures.boxTexDescriptorSet
			  , sceneData->textures.boxRTexDescriptorSet
			  , sceneData->textures.boxlightTexDescriptorSet
			  , sceneData->textures.boxlightRTexDescriptorSet
			  , sceneData->textures.bombTexDescriptorSet
			  , sceneData->textures.bombRTexDescriptorSet
			  , sceneData->textures.balloonTexDescriptorSet
			  , sceneData->textures.balloonRTexDescriptorSet
			  , sceneData->textures.balloonMoveTexDescriptorSet
			  , sceneData->textures.balloonMoveRTexDescriptorSet
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
		  , uiFrameState()
		  //, timeEngine(nullptr)
		{
		}
		VulkanDataKeepAlive getSharedVulkanData() override {
			return VulkanDataKeepAlive{sceneData};
		}
		VulkanDataKeepAlive getFrameVulkanData(std::size_t const currentFrame) override {
			return VulkanDataKeepAlive{frameData[currentFrame]};
		}
		void updateSwapChainData(
			VkRenderPass const renderPass,
			VkExtent2D const &swapChainExtent) override
		{
			this->renderPass = renderPass;
			this->swapChainExtent = swapChainExtent;
			sceneData->renderTargets.clear();
			sceneData->textures.timelineTextures.clear();
			{
				auto const res{vkResetDescriptorPool(device, sceneData->textures.timelineTextureDescriptorPool.descriptorPool, 0)};
				if (res != VK_SUCCESS) {
					BOOST_THROW_EXCEPTION(std::system_error(res, "timelineTextureDescriptorPool vkResetDescriptorPool failed"));
				}
			}
			sceneData->graphicsPipeline = VulkanGraphicsPipelineHG(device);
			//pipelineLayout = VulkanPipelineLayout(device);

			sceneData->textures.timelineTextures = createTimelineTextures(physicalDevice, device, gsl::narrow<int>(getTimelineTextureWidth(swapChainExtent)), gsl::narrow<int>(getTimelineTextureHeight()), sceneData->textures.timelineTextureDescriptorPool.descriptorPool, sceneData->textureDescriptorSetLayout.descriptorSetLayout);

			//pipelineLayout = VulkanPipelineLayout(device, makePipelineLayoutCreateInfo(descriptorSetLayout.descriptorSetLayout));
			sceneData->graphicsPipeline = VulkanGraphicsPipelineHG(device, swapChainExtent, sceneData->pipelineLayout.pipelineLayout, renderPass);
			sceneData->renderTargets = createRenderTargets(physicalDevice, device, sceneData->pipelineLayout.pipelineLayout, sceneData->projUniformDescriptorSetLayout.descriptorSetLayout, sceneData->preDrawCommandBuffers, sceneData->drawCommandBuffers);
		}
		std::vector<VkCommandBuffer> renderFrame(
			std::size_t currentFrame,//TODO: Find a better way to manage the lifetimes of things that must be kept alive for the duration of a frame render!
			VkFramebuffer const targetFrameBuffer) override
		{
			auto const &preDrawCommandBuffer{ sceneData->preDrawCommandBuffers[currentFrame] };
			auto const &drawCommandBuffer{ sceneData->drawCommandBuffers[currentFrame] };
			//auto const samplerDescriptorPool{samplerDescriptorPools[currentFrame].descriptorPool};
			{
				{
					auto const res{vkResetCommandBuffer(preDrawCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)};
					if (res != VK_SUCCESS) {
						BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't reset pre-draw command buffer!"));
					}
				}
				{
					auto const res{vkResetCommandBuffer(drawCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)};
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
					auto const res{vkBeginCommandBuffer(preDrawCommandBuffer, &beginInfo)};
					if (res != VK_SUCCESS) {
						BOOST_THROW_EXCEPTION(std::system_error(res, "failed to begin recording preDraw command buffer!"));
					}
				}
				{
					auto const res{vkBeginCommandBuffer(drawCommandBuffer, &beginInfo)};
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
					auto const res{vkEndCommandBuffer(drawCommandBuffer)};
					if (res != VK_SUCCESS) {
						BOOST_THROW_EXCEPTION(std::system_error(res, "failed to record draw command buffer!"));
					}
				}
				{
					auto const res{vkEndCommandBuffer(preDrawCommandBuffer)};
					if (res != VK_SUCCESS) {
						BOOST_THROW_EXCEPTION(std::system_error(res, "failed to record preDraw command buffer!"));
					}
				}
			}
			return {preDrawCommandBuffer, drawCommandBuffer};
		}
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
			vkCmdBindPipeline(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->graphicsPipeline.h());

			reallyDoRender(preDrawCommandBuffer, drawCommandBuffer, target, targetFrameBuffer, timelineTexture);
			
			vkCmdEndRenderPass(drawCommandBuffer);
			target.flushBuffersToDevice();
		}
		void setUiFrameState(UIFrameState &&newUiFrameState){
			std::lock_guard<std::mutex> lock{uiFrameStateMutex};
			uiFrameState = std::make_unique<UIFrameState>(std::move(newUiFrameState));
		}
		std::optional<UIFrameState> copyUiFrameState(){
			std::lock_guard<std::mutex> lock{uiFrameStateMutex};
			return uiFrameState ? std::optional<UIFrameState>(*uiFrameState) : std::optional<UIFrameState>{};
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
				uiFrameStateLocal->drawnGlitz/*getGlitzForDirection(timeEngine->getFrame(uiFrameState->drawnFrame)->getView(), uiFrameState->drawnTimeDirection)*/,
				uiFrameStateLocal->wall/*timeEngine->getWall()*/,
				static_cast<int>(uiFrameStateLocal->guyIndex),
				targetFrameBuffer,
				drawCommandBuffer,
				swapChainExtent,
				texDescriptorSets,
				sceneData->pipelineLayout.pipelineLayout,
				sceneData->textures);

			DrawInventory(
				target,
				drawCommandBuffer,
				uiFrameStateLocal->pickups,
				uiFrameStateLocal->shouldDrawInventory,
				uiFrameStateLocal->paused,
				uiFrameStateLocal->input.getAbilityCursor());

			DrawParadoxPressure(
				target,
				drawCommandBuffer,
				uiFrameStateLocal->waveInfo.paradoxPressure,
				uiFrameStateLocal->waveInfo.minWaveChanges);

			DrawTimeline(
				target,
				preDrawCommandBuffer,
				drawCommandBuffer,
				timelineTexture,
				uiFrameStateLocal->timelineLength,
				uiFrameStateLocal->waveInfo.updatedFrames,
				uiFrameStateLocal->drawnFrame,
				uiFrameStateLocal->input.getTimeCursor(),
				uiFrameStateLocal->guyFrames,
				uiFrameStateLocal->wall,
				swapChainExtent,
				sceneData->pipelineLayout.pipelineLayout);

			//DrawPersonalTimeline(
			//	target,
			//	uiFrameStateLocal->wall,
			//	drawCommandBuffer,
			//	uiFrameStateLocal->relativeGuyIndex,
			//	uiFrameStateLocal->guyFrames,
			//	uiFrameStateLocal->postOverwriteInput,
			//	uiFrameStateLocal->timelineLength,
			//	swapChainExtent);

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
						//Out  x	y	z	v
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
				std::stringstream timeString;
				timeString << "Time: " << (uiFrameStateLocal->drawnFrame.getFrameNumber() * 10 / hg::FRAMERATE) / 10. << "s";
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
				std::stringstream currentPlayerIndex;
				currentPlayerIndex << "Age: " << ((std::size(uiFrameStateLocal->postOverwriteInput) - 1)  * 10 / hg::FRAMERATE) / 10. << "s";

				hg::drawText(
					target,
					drawCommandBuffer,
					sceneData->pipelineLayout.pipelineLayout,
					texDescriptorSets.fontTexDescriptorSet,
					currentPlayerIndex.str(),
					98.f,
					static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) + 45.f,
					16.f,
					UI_TEXT_COLOR);
			}
			{
				std::stringstream frameNumberString;
				frameNumberString << "Frame: " << uiFrameStateLocal->drawnFrame.getFrameNumber();
				hg::drawText(
					target,
					drawCommandBuffer,
					sceneData->pipelineLayout.pipelineLayout,
					texDescriptorSets.fontTexDescriptorSet,
					frameNumberString.str(),
					90.f,
					static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) + 100.f,
					16.f,
					UI_TEXT_COLOR);
			}
			{
				std::vector<int> framesExecutedList;
				framesExecutedList.reserve(boost::size(uiFrameStateLocal->waveInfo.updatedFrames));
				for (
					hg::FrameUpdateSet const& updateSet :
					uiFrameStateLocal->waveInfo.updatedFrames)
				{
					framesExecutedList.push_back(static_cast<int>(boost::size(updateSet)));
				}
				std::stringstream numberOfFramesExecutedString;
				if (!boost::empty(framesExecutedList)) {
					numberOfFramesExecutedString << *boost::begin(framesExecutedList);
					for (
						int num :
					framesExecutedList
						| boost::adaptors::sliced(1, boost::size(framesExecutedList)))
					{
						numberOfFramesExecutedString << ":" << num;
					}
				}
				hg::drawText(
					target,
					drawCommandBuffer,
					sceneData->pipelineLayout.pipelineLayout,
					texDescriptorSets.fontTexDescriptorSet,
					numberOfFramesExecutedString.str(),
					10.f,
					826.f,
					12.f,
					UI_TEXT_COLOR);
			}
			//TODO: Rething FPS measurements, now that
			//GUI and TimeEngine frames can be independent
			//(in the near future they will be)

			if (uiFrameStateLocal->runningFromReplay) {
				//TODO: also write some sort of replay progress display here
				//currentReplayIt-replay.begin() << "/" << currentReplayEnd-replay.begin()

				hg::drawText(
					target,
					drawCommandBuffer,
					sceneData->pipelineLayout.pipelineLayout,
					texDescriptorSets.fontTexDescriptorSet,
					"R",
					580.f,
					32.f,
					32.f,
					vec3<float>{ 255.f/255.f, 25.f / 255.f, 50.f / 255.f });
			}
		}


		void DrawInventory(
			VulkanRenderTarget& target,
			VkCommandBuffer const& drawCommandBuffer,
			Pickups const& pickups,
			bool shouldDrawItems,
			bool paused,
			Ability abilityCursor)
		{
			Pickups mpickups(pickups);
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
						//Out  x	y	z	v
						  a, 0.0, 0.0, 0.0,//In x
						0.0,   b, 0.0, 0.0,//In y
						0.0, 0.0, 1.0, 0.0,//In z
						  c,   d, 0.0, 1.0 //In v
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
			
			float drawPos = static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) - BUTTON_AREA_HEIGHT;
			
			{
				drawRect(target, 
					static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_PAUSE_X,
					drawPos,
					static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_PAUSE_WIDTH,
					BUTTON_HEIGHT,
					paused ? BUTTON_ACTIVE : BUTTON_INACTIVE, 0);
					
				std::stringstream pause;
				pause << "Pause";
				hg::drawText(
					target,
					drawCommandBuffer,
					sceneData->pipelineLayout.pipelineLayout,
					texDescriptorSets.fontTexDescriptorSet,
					pause.str(),
					static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.40f,
					drawPos + 6,
					16.f,
					BUTTON_TEXT_COLOR);
				
				drawPos = drawPos + BUTTON_PAUSE_SPACING;
			}
			
			if (shouldDrawItems) {
				{
					drawRect(target, 
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_X,
						drawPos,
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_WIDTH,
						BUTTON_HEIGHT,
						abilityCursor == Ability::TIME_JUMP ? BUTTON_ACTIVE : BUTTON_INACTIVE, 0);
					std::stringstream timeJump;
					timeJump << "1) timeJumps: " << mpickups[Ability::TIME_JUMP];
					hg::drawText(
						target,
						drawCommandBuffer,
						sceneData->pipelineLayout.pipelineLayout,
						texDescriptorSets.fontTexDescriptorSet,
						timeJump.str(),
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.3f - 30.f,
						drawPos + 6,
						16.f,
						BUTTON_TEXT_COLOR);
					drawPos = drawPos + BUTTON_SPACING;
				}
				{
					drawRect(target, 
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_X,
						drawPos,
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_WIDTH,
						BUTTON_HEIGHT,
						abilityCursor == Ability::TIME_REVERSE ? BUTTON_ACTIVE : BUTTON_INACTIVE, 0);
					std::stringstream timeReverses;
					timeReverses << "2) timeReverses: " << mpickups[Ability::TIME_REVERSE];
					hg::drawText(
						target,
						drawCommandBuffer,
						sceneData->pipelineLayout.pipelineLayout,
						texDescriptorSets.fontTexDescriptorSet,
						timeReverses.str(),
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.3f - 30.f,
						drawPos + 6,
						16.f,
						BUTTON_TEXT_COLOR);
					drawPos = drawPos + BUTTON_SPACING;
				}
				{
					drawRect(target, 
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_X,
						drawPos,
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_WIDTH,
						BUTTON_HEIGHT,
						abilityCursor == Ability::TIME_GUN ? BUTTON_ACTIVE : BUTTON_INACTIVE, 0);
					std::stringstream timeGuns;
					timeGuns << "3) timeGuns: " << mpickups[Ability::TIME_GUN];
					hg::drawText(
						target,
						drawCommandBuffer,
						sceneData->pipelineLayout.pipelineLayout,
						texDescriptorSets.fontTexDescriptorSet,
						timeGuns.str(),
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.3f - 30.f,
						drawPos + 6,
						16.f,
						BUTTON_TEXT_COLOR);
					drawPos = drawPos + BUTTON_SPACING;
				}
				{
					drawRect(target, 
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_X,
						drawPos,
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*BUTTON_WIDTH,
						BUTTON_HEIGHT,
						abilityCursor == Ability::TIME_PAUSE ? BUTTON_ACTIVE : BUTTON_INACTIVE, 0);
					std::stringstream timePauses;
					timePauses << "4) timePauses: " << mpickups[Ability::TIME_PAUSE];
					hg::drawText(
						target,
						drawCommandBuffer,
						sceneData->pipelineLayout.pipelineLayout,
						texDescriptorSets.fontTexDescriptorSet,
						timePauses.str(),
						static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.3f - 30.f,
						drawPos + 6,
						16.f,
						BUTTON_TEXT_COLOR);
					drawPos = drawPos + BUTTON_SPACING;
				}
			}
		}


		void DrawParadoxPressure(
			VulkanRenderTarget& target,
			VkCommandBuffer const& drawCommandBuffer,
			int const paradoxPressure,
			int const minWaveChanges)
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
					//Out  x	y	z	v
						a, 0.0, 0.0, 0.0,//In x
					0.0,   b, 0.0, 0.0,//In y
					0.0, 0.0, 1.0, 0.0,//In z
						c,   d, 0.0, 1.0 //In v
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

			float top = static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y)*0.1f;
			float bottom = static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) - 315.f;
			float pressureProportion = std::min(1.f, static_cast<float>(paradoxPressure) / static_cast<float>(hg::PARADOX_PRESSURE_MAX));

			vec3<float> backgroundColor{ 50.f / 255.f, 50.f / 255.f, 50.f / 255.f };
			vec3<float> pressureColor{ (200.f + 50.f * pressureProportion) / 255.f, 200.f / 255.f, 200.f / 255.f };

			drawRect(target,
				static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.35f,
				top,
				static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.3f,
				bottom - top,
				backgroundColor, 0);
			drawRect(target,
				static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.35f,
				top + (bottom - top)*(1.f - pressureProportion),
				static_cast<float>(hg::WINDOW_DEFAULT_X*hg::UI_DIVIDE_X)*0.3f,
				(bottom - top)*pressureProportion,
				pressureColor, 0);

			std::stringstream pressureStr;
			pressureStr << "Pressure: " << static_cast<int>(std::round(100.f * pressureProportion)) << "%";
			hg::drawText(
				target,
				drawCommandBuffer,
				sceneData->pipelineLayout.pipelineLayout,
				texDescriptorSets.fontTexDescriptorSet,
				pressureStr.str(),
				20.f,
				static_cast<float>(hg::WINDOW_DEFAULT_Y * hg::UI_DIVIDE_Y) - 300.f,
				16.f,
				UI_TEXT_COLOR);
		}

		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkExtent2D swapChainExtent;
		VkRenderPass renderPass;

		std::shared_ptr<RunningGameSceneSharedVulkanData> sceneData;
		std::vector<std::shared_ptr<RunningGameSceneFrameVulkanData>> frameData;

		GameDisplayTexDescriptorSets texDescriptorSets;

		private:
		std::mutex uiFrameStateMutex;
		std::unique_ptr<UIFrameState> uiFrameState;
	};
}
#endif // !HG_RUNNING_GAME_SCENE_RENDERER_H
