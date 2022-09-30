#include "SelectionScene.h"
#include "RunningGameSceneRenderer.h"
#include "hg/Util/unlock_util.h"

namespace hg {

	struct SelectionSceneSharedVulkanData {
		explicit SelectionSceneSharedVulkanData(
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
		, fontTex("unifont.png", physicalDevice.physicalDevice, device, commandPool.h(), graphicsQueue, true)
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
	struct SelectionSceneFrameVulkanData {

	};

	class SelectionSceneRenderer : public SceneRenderer {
	public:
		explicit SelectionSceneRenderer(
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
				 std::make_shared<SelectionSceneSharedVulkanData>(
					 physicalDevice,
					 device,
					 surface,
					 renderPass,
					 swapChainExtent,
					 graphicsQueue)
			 )
		   , frameData([] {
				 std::vector<std::shared_ptr<SelectionSceneFrameVulkanData>> frameData;
				 for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
					 frameData.emplace_back(std::make_shared<SelectionSceneFrameVulkanData>());
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
		void setUiFrameState(std::string&& newSelectionSceneUiFrameState) {
			std::lock_guard lock{ selectionSceneUiFrameStateMutex };
			selectionSceneUiFrameState = std::make_unique<std::string>(std::move(newSelectionSceneUiFrameState));
		}
	private:

		void doRender(
			VkCommandBuffer const& preDrawCommandBuffer,
			VkCommandBuffer const& drawCommandBuffer,
			VulkanRenderTarget& target,
			VkFramebuffer const targetFrameBuffer)
		{
			//TODO: don't duplicate this logic with RunningGameSceneRenderer::doRender and MainMenuSceneRenderer
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

			drawText(target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet, *uiFrameStateLocal, 400.f, 200.f, 32.f, vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f });
		}
		std::optional<std::string> copyUiFrameState() {
			std::lock_guard lock{ selectionSceneUiFrameStateMutex };
			return selectionSceneUiFrameState ? std::optional(*selectionSceneUiFrameState) : std::optional<std::string>{};
		}

		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkExtent2D swapChainExtent;
		VkRenderPass renderPass;

		std::shared_ptr<SelectionSceneSharedVulkanData> sceneData;
		std::vector<std::shared_ptr<SelectionSceneFrameVulkanData>> frameData;


		mutable std::mutex selectionSceneUiFrameStateMutex;
		std::unique_ptr<std::string> selectionSceneUiFrameState;
	};

	class SelectionPageSceneRenderer : public SceneRenderer {
	public:
		explicit SelectionPageSceneRenderer(
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
				 std::make_shared<SelectionSceneSharedVulkanData>(
					 physicalDevice,
					 device,
					 surface,
					 renderPass,
					 swapChainExtent,
					 graphicsQueue)
			 )
		   , frameData([] {
				 std::vector<std::shared_ptr<SelectionSceneFrameVulkanData>> frameData;
				 for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
					 frameData.emplace_back(std::make_shared<SelectionSceneFrameVulkanData>());
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
		void setUiFrameState(int selectedItem, int selectedPage, std::vector<PageState> const &levelMenuConf, bool unlockAll) {
			std::lock_guard lock{ selectionSceneUiFrameStateMutex };
			selectionPageSceneUiFrameState = std::make_unique<SelectionPageFrameState>(selectedItem, selectedPage, std::move(levelMenuConf), unlockAll);
		}
	private:

		void doRender(
			VkCommandBuffer const& preDrawCommandBuffer,
			VkCommandBuffer const& drawCommandBuffer,
			VulkanRenderTarget& target,
			VkFramebuffer const targetFrameBuffer)
		{
			//TODO: don't duplicate this logic with RunningGameSceneRenderer::doRender and MainMenuSceneRenderer
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

			float drawPos = 150.f;
			int optPos = 0;
			int selectedItem = (*uiFrameStateLocal).selectedItem;
			
			PageState pageInfo = (*uiFrameStateLocal).pages[(*uiFrameStateLocal).page];
			
			int completedOnPrevPages = 0;
			for (size_t p = 0; p < (*uiFrameStateLocal).page; ++p) {
				for (auto it = ((*uiFrameStateLocal).pages[p].options).begin(); it != ((*uiFrameStateLocal).pages[p].options).end(); ++it) {
					if (IsLevelComplete((*it).name)) {
						completedOnPrevPages += 1;
					}
				}
			}
			if (completedOnPrevPages < pageInfo.prevLevelsRequired && !(*uiFrameStateLocal).unlockAll) {
				drawText(
					target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet,
					pageInfo.name + " (Locked, " + std::to_string(completedOnPrevPages) + " / " + std::to_string(pageInfo.prevLevelsRequired) + " levels required)",
					400.f, drawPos, 32.f, 
					vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f });
				drawPos += 42.f;
			} else {
				drawText(
					target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet,
					pageInfo.name, 400.f, drawPos, 32.f, 
					vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f });
				drawPos += 42.f;
				
				int completedLevels = 0;
				for (auto it = (pageInfo.options).begin(); it != (pageInfo.options).end(); ++it, ++optPos) {
					if (IsLevelComplete((*it).name)) {
						completedLevels += 1;
					}
					bool unlocked = (*uiFrameStateLocal).unlockAll || IsLevelComplete((*it).name);
					if (!unlocked) {
						if (std::holds_alternative<int>((*it).unlockRequirement)) {
							unlocked = (completedLevels >= std::get<int>((*it).unlockRequirement));
						} else {
							unlocked = IsLevelComplete(std::get<std::string>((*it).unlockRequirement));
						}
					}
					if (unlocked) {
						drawText(
							target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet,
							(IsLevelComplete((*it).name) ? (*it).humanName + std::string(" (done)") : (*it).humanName),
							400.f, drawPos, 32.f, 
							(selectedItem == optPos ? 
								((*uiFrameStateLocal).unlockAll || IsLevelComplete((*it).name) ?
									vec3<float>{ 128.f / 255.f, 255.f / 255.f, 255.f / 255.f } :
									vec3<float>{  90.f / 255.f, 180.f / 255.f, 180.f / 255.f } ) :
								((*uiFrameStateLocal).unlockAll || IsLevelComplete((*it).name) ?
									vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f } :
									vec3<float>{ 160.f / 255.f, 160.f / 255.f, 160.f / 255.f } )));
						drawPos += 42.f;
					}
				}
			}
		}
		std::optional<SelectionPageFrameState> copyUiFrameState() {
			std::lock_guard lock{ selectionSceneUiFrameStateMutex };
			return selectionPageSceneUiFrameState ? std::optional(*selectionPageSceneUiFrameState) : std::optional<SelectionPageFrameState>();
			//return selectionPageSceneUiFrameState ? std::optional(*selectionPageSceneUiFrameState) : std::optional<std::vector<std::string>>{};
		}

		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkExtent2D swapChainExtent;
		VkRenderPass renderPass;

		std::shared_ptr<SelectionSceneSharedVulkanData> sceneData;
		std::vector<std::shared_ptr<SelectionSceneFrameVulkanData>> frameData;


		mutable std::mutex selectionSceneUiFrameStateMutex;
		std::unique_ptr<SelectionPageFrameState> selectionPageSceneUiFrameState;
	};

	std::variant<std::size_t, SceneAborted_tag> run_selection_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		std::vector<std::string> const &options,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer)
	{
		SelectionSceneRenderer renderer{
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

		if (options.empty()) //If no options available, just display a blank screen until the user escapes out.
		{
			//window.clear();
			//window.display();
			while (true) {
				glfwPollEvents();
				if (glfwWindowShouldClose(windowglfw.w)) {
					//window.close();
					throw WindowClosed_exception{};
				}

				if (windowglfw.hasLastKey()) {
					int key = windowglfw.useLastKey();
					if (key == GLFW_KEY_ESCAPE) {
						return SceneAborted_tag{};
					}
				}
			}
		}

		//TODO: Use size_t rather than int.
		int selectedItem = defaultOption;

		while (true) {
			renderer.setUiFrameState(std::string{ options[selectedItem] });
			//drawOptionSelection(window, options[selectedItem]);
			bool menuDrawn = true;
			while (menuDrawn) {
				glfwPollEvents();

				if (glfwWindowShouldClose(windowglfw.w)) {
					//window.close();
					throw WindowClosed_exception{};
				}

				if (windowglfw.hasLastKey()) {
					int key = windowglfw.useLastKey();
					if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
						return static_cast<std::size_t>(selectedItem);
					}
					if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
						selectedItem = flooredModulo(selectedItem - 1, static_cast<int>(options.size()));
						menuDrawn = false;
					}
					if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
						selectedItem = flooredModulo(selectedItem + 1, static_cast<int>(options.size()));
						menuDrawn = false;
					}
					if (key == GLFW_KEY_ESCAPE) {
						return SceneAborted_tag{};
					}
				}
			}
		}
	}

	std::variant<LevelSelectionReturn, SceneAborted_tag> run_selection_page_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		int defaultPage,
		std::vector<PageState> const &levelMenuConf,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer,
		bool unlockAll)
	{
		SelectionPageSceneRenderer renderer{
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

		if (levelMenuConf.empty()) //If no pages available, just display a blank screen until the user escapes out.
		{
			//window.clear();
			//window.display();
			while (true) {
				glfwPollEvents();
				if (glfwWindowShouldClose(windowglfw.w)) {
					//window.close();
					throw WindowClosed_exception{};
				}

				if (windowglfw.hasLastKey()) {
					int key = windowglfw.useLastKey();
					if (key == GLFW_KEY_ESCAPE) {
						return SceneAborted_tag{};
					}
				}
			}
		}

		//TODO: Use size_t rather than int.
		int selectedItem = defaultOption;
		int selectedPage = defaultPage;

		while (true) {
			//std::cout << "page: " << std::to_string(page) << ", selectedItem: " << std::to_string(selectedItem) << ", perPage: " << std::to_string(perPage) << "\n" << std::flush;
			renderer.setUiFrameState(selectedItem, selectedPage, levelMenuConf, unlockAll);
			//drawOptionSelection(window, levelMenuConf[selectedItem]);
			bool menuDrawn = true;
			while (menuDrawn) {
				glfwPollEvents();

				if (glfwWindowShouldClose(windowglfw.w)) {
					//window.close();
					throw WindowClosed_exception{};
				}
				
				if (windowglfw.hasLastKey()) {
					int key = windowglfw.useLastKey();
					
					int completedLevels = 0;
					for (auto it = (levelMenuConf[selectedPage].options).begin(); it != (levelMenuConf[selectedPage].options).end(); ++it) {
						if (IsLevelComplete((*it).name)) {
							completedLevels += 1;
						}
					}
					
					if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
						int completedOnPrevPages = 0;
						for (size_t p = 0; p < selectedPage; ++p) {
							for (auto it = (levelMenuConf[p].options).begin(); it != (levelMenuConf[p].options).end(); ++it) {
								if (IsLevelComplete((*it).name)) {
									completedOnPrevPages += 1;
								}
							}
						}
						if (completedOnPrevPages >= levelMenuConf[selectedPage].prevLevelsRequired) {
							bool unlocked = unlockAll || IsLevelComplete(levelMenuConf[selectedPage].options[selectedItem].name);
							if (!unlocked) {
								if (std::holds_alternative<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement)) {
									unlocked = (completedLevels >= std::get<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								} else {
									unlocked = IsLevelComplete(std::get<std::string>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								}
							}
							if (unlocked) {
								return LevelSelectionReturn(levelMenuConf[selectedPage].options[selectedItem].name, selectedItem, selectedPage);
							}
						}
					}
					if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
						int tries = static_cast<int>(levelMenuConf[selectedPage].options.size());
						bool unlocked = false;
						do {
							selectedItem = flooredModulo(selectedItem - 1, static_cast<int>(levelMenuConf[selectedPage].options.size()));
							unlocked = unlockAll || IsLevelComplete(levelMenuConf[selectedPage].options[selectedItem].name);
							if (!unlocked) {
								if (std::holds_alternative<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement)) {
									unlocked = (completedLevels >= std::get<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								} else {
									unlocked = IsLevelComplete(std::get<std::string>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								}
							}
							tries -= 1;
						} while (!unlocked && tries);
						menuDrawn = false;
					}
					if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
						int tries = static_cast<int>(levelMenuConf[selectedPage].options.size());
						bool unlocked = false;
						do {
							selectedItem = flooredModulo(selectedItem + 1, static_cast<int>(levelMenuConf[selectedPage].options.size()));
							unlocked = unlockAll || IsLevelComplete(levelMenuConf[selectedPage].options[selectedItem].name);
							if (!unlocked) {
								if (std::holds_alternative<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement)) {
									unlocked = (completedLevels >= std::get<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								} else {
									unlocked = IsLevelComplete(std::get<std::string>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								}
							}
							tries -= 1;
						} while (!unlocked && tries);
						menuDrawn = false;
					}
					
					bool needPageFix = false;
					if (key == GLFW_KEY_LEFT|| key == GLFW_KEY_A) {
						if (selectedPage > 0) {
							selectedPage = selectedPage - 1;
							menuDrawn = false;
							needPageFix = true;
						}
					}
					if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
						if (selectedPage < static_cast<int>(levelMenuConf.size()) - 1) {
							selectedPage = selectedPage + 1;
							menuDrawn = false;
							needPageFix = true;
						}
					}
					if (key == GLFW_KEY_ESCAPE) {
						return SceneAborted_tag{};
					}
					
					// Fix level out of bounds on change
					if (needPageFix) {
						int tries = static_cast<int>(levelMenuConf[selectedPage].options.size());
						if (selectedItem >= tries) {
							selectedItem = tries;
						}
						bool unlocked = false;
						selectedItem += 1; // Lazy counteraction for do loop.
						do {
							selectedItem = flooredModulo(selectedItem - 1, static_cast<int>(levelMenuConf[selectedPage].options.size()));
							unlocked = unlockAll || IsLevelComplete(levelMenuConf[selectedPage].options[selectedItem].name);
							if (!unlocked) {
								if (std::holds_alternative<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement)) {
									unlocked = (completedLevels >= std::get<int>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								} else {
									unlocked = IsLevelComplete(std::get<std::string>(levelMenuConf[selectedPage].options[selectedItem].unlockRequirement));
								}
							}
							tries -= 1;
						} while (!unlocked && tries);
					}
				}
			}
		}
	}
}
