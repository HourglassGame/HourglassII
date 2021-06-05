#include "MainMenuScene.h"
#include "hg/Util/Maths.h"
#include <chrono>
#include <iostream>
#include "VulkanRenderer.h"
namespace hg {
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

		, samplerDescriptorPool(createSamplerDescriptorPool(device))
		//, samplerDescriptorPools(createSamplerDescriptorPools(device))
		, renderTargets(createRenderTargets(physicalDevice.physicalDevice, device, pipelineLayout.pipelineLayout, projUniformDescriptorSetLayout.descriptorSetLayout, preDrawCommandBuffers, drawCommandBuffers))

		//, timelineTextureDescriptorPool(createTimelineTextureDescriptorPool(device))
		//, timelineTextures(createTimelineTextures(physicalDevice, device, gsl::narrow<int>(getTimelineTextureWidth(swapChainExtent)), gsl::narrow<int>(getTimelineTextureHeight()), timelineTextureDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout))
		, fontTex("unifont.png", physicalDevice.physicalDevice, device, commandPool.h(), graphicsQueue, true)
		, fontTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, fontTex))
#if 0
		, boxTex("GlitzData/box.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, boxTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, boxTex))

		, boxRTex("GlitzData/box_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, boxRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, boxRTex))

		, boxlightTex("GlitzData/boxlight.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, boxlightTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, boxlightTex))

		, boxlightRTex("GlitzData/boxlight_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, boxlightRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, boxlightRTex))

		, bombTex("GlitzData/bomb.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, bombTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, bombTex))

		, bombRTex("GlitzData/bomb_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, bombRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, bombRTex))

		, balloonTex("GlitzData/balloon.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, balloonTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, balloonTex))

		, balloonRTex("GlitzData/balloon_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, balloonRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, balloonRTex))

		, balloonMoveTex("GlitzData/balloon_move.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, balloonMoveTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, balloonMoveTex))

		, balloonMoveRTex("GlitzData/balloon_move_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, balloonMoveRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, balloonMoveRTex))

		, powerupJumpTex("GlitzData/powerup_jump.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, powerupJumpTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, powerupJumpTex))

		, rhinoLeftStopTex("GlitzData/rhino_left_stop.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, rhinoLeftStopTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoLeftStopTex))

		, rhinoLeftStopRTex("GlitzData/rhino_left_stop_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, rhinoLeftStopRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoLeftStopRTex))

		, rhinoRightStopTex("GlitzData/rhino_right_stop.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, rhinoRightStopTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoRightStopTex))

		, rhinoRightStopRTex("GlitzData/rhino_right_stop_r.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, rhinoRightStopRTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, rhinoRightStopRTex))

		, timeGunTex("GlitzData/time_gun.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, timeGunTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timeGunTex))

		, timeJumpTex("GlitzData/time_jump.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, timeJumpTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timeJumpTex))

		, timePauseTex("GlitzData/time_pause.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, timePauseTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timePauseTex))

		, timeReverseTex("GlitzData/time_reverse.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, timeReverseTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, timeReverseTex))

		, trampolineTex("GlitzData/trampoline.png", device, physicalDevice, commandPool.h(), graphicsQueue, false)
		, trampolineTexDescriptorSet(createDescriptorSet(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, trampolineTex))

		, wallBlockImages(loadWallBlockImages(device, physicalDevice, commandPool.h(), graphicsQueue))
		, wallBlockDescriptorSets(loadWallBlockDescriptorSets(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, wallBlockImages))
		, wallCornerImages(loadWallCornerImages(device, physicalDevice, commandPool.h(), graphicsQueue))
		, wallCornerDescriptorSets(loadWallCornerDescriptorSets(device, samplerDescriptorPool.descriptorPool, textureDescriptorSetLayout.descriptorSetLayout, wallCornerImages))
		, texDescriptorSets(TexDescriptorSets{
				  fontTexDescriptorSet
				, boxTexDescriptorSet
				, boxRTexDescriptorSet
				, boxlightTexDescriptorSet
				, boxlightRTexDescriptorSet
				, bombTexDescriptorSet
				, bombRTexDescriptorSet
				, balloonTexDescriptorSet
				, balloonRTexDescriptorSet
				, balloonMoveTexDescriptorSet
				, balloonMoveRTexDescriptorSet
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

	VulkanCommandPoolHG commandPool;
	std::vector<VkCommandBuffer> preDrawCommandBuffers;
	std::vector<VkCommandBuffer> drawCommandBuffers;

	VulkanDescriptorSetLayout projUniformDescriptorSetLayout;
	VulkanDescriptorSetLayout textureDescriptorSetLayout;
	VulkanPipelineLayout pipelineLayout;
	VulkanGraphicsPipelineHG graphicsPipeline;
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

	VulkanTextureSimple boxlightTex;
	VkDescriptorSet boxlightTexDescriptorSet;

	VulkanTextureSimple boxlightRTex;
	VkDescriptorSet boxlightRTexDescriptorSet

	VulkanTextureSimple bombTex;
	VkDescriptorSet bombTexDescriptorSet;

	VulkanTextureSimple bombRTex;
	VkDescriptorSet bombRTexDescriptorSet;

	VulkanTextureSimple balloonTex;
	VkDescriptorSet balloonTexDescriptorSet;

	VulkanTextureSimple balloonRTex;
	VkDescriptorSet balloonRTexDescriptorSet;

	VulkanTextureSimple balloonMoveTex;
	VkDescriptorSet balloonMoveTexDescriptorSet;

	VulkanTextureSimple balloonMoveRTex;
	VkDescriptorSet balloonMoveRTexDescriptorSet;

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
		sceneData->graphicsPipeline = VulkanGraphicsPipelineHG(device);
		//pipelineLayout = VulkanPipelineLayout(device);

		//pipelineLayout = VulkanPipelineLayout(device, makePipelineLayoutCreateInfo(descriptorSetLayout.descriptorSetLayout));
		sceneData->graphicsPipeline = VulkanGraphicsPipelineHG(device, swapChainExtent, sceneData->pipelineLayout.pipelineLayout, renderPass);
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
		vkCmdBindPipeline(drawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->graphicsPipeline.h());

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
					//Out  x	y	z	v
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

std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag>
run_main_menu(
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
		vulkanEng.device.h(),
		vulkanEng.surface.surface,
		vulkanEng.renderPass.h(),
		//eng.graphicsPipeline.graphicsPipeline,
		//eng.pipelineLayout.pipelineLayout,
		//eng.descriptorSetLayout.descriptorSetLayout,
		vulkanEng.swapChain.extent(),
		vulkanEng.device.graphicsQ()
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

		bool mainMenuDrawn = true;
		while (mainMenuDrawn) {
			glfwPollEvents();

			//vulkanEng.drawFrame();
			auto nextFrame = std::chrono::steady_clock::now();
			std::chrono::duration<double> frameTime = nextFrame - frameStart;
			//std::cout << std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count() << "\n";
			frameStart = nextFrame;

			if (glfwWindowShouldClose(windowglfw.w)) {
				//std::cout << "Main Menu windowglfw Pointer: " << &windowglfw << "\n" << std::flush;
				//std::cout << "Main Menu windowglfw.w Pointer: " << (windowglfw.w) << "\n" << std::flush;
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
