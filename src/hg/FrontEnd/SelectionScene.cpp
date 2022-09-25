#include "SelectionScene.h"
#include "RunningGameSceneRenderer.h"
namespace hg {

	struct SelectionPageFrameState {
		int selectedItem;
		int page;
		int perPage;
		std::vector<PageState> pages;
	};

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
		void setUiFrameState(int selectedItem, int page, int perPage, std::vector<PageState> const &levelMenuConf) {
			std::lock_guard lock{ selectionSceneUiFrameStateMutex };
			selectionPageSceneUiFrameState = std::make_unique<SelectionPageFrameState>(selectedItem, page, perPage, std::move(levelMenuConf));
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
			int itemMin = (*uiFrameStateLocal).page * (*uiFrameStateLocal).perPage;
			int itemMax = itemMin + (*uiFrameStateLocal).perPage;
			int selectedItem = (*uiFrameStateLocal).selectedItem;
			for (auto it = ((*uiFrameStateLocal).pages).begin(); it != ((*uiFrameStateLocal).pages).end(); ++it, ++optPos) {
				//if (optPos >= itemMin && optPos < itemMax) {
				//	drawText(
				//		target, drawCommandBuffer, sceneData->pipelineLayout.pipelineLayout, sceneData->fontTexDescriptorSet,
				//		*it, 400.f, drawPos, 32.f, 
				//		(selectedItem == optPos ? vec3<float>{ 128.f / 255.f, 255.f / 255.f, 255.f / 255.f } : vec3<float>{ 255.f / 255.f, 255.f / 255.f, 255.f / 255.f }));
				//	drawPos += 42.f;
				//}
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

	std::variant<std::size_t, SceneAborted_tag> run_selection_page_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		int perPage,
		std::vector<PageState> const &levelMenuConf,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer)
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

		while (true) {
			int page = selectedItem / perPage;
			//std::cout << "page: " << std::to_string(page) << ", selectedItem: " << std::to_string(selectedItem) << ", perPage: " << std::to_string(perPage) << "\n" << std::flush;
			renderer.setUiFrameState(selectedItem, page, perPage, levelMenuConf);
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
					if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
						return static_cast<std::size_t>(selectedItem);
					}
					if (key == GLFW_KEY_UP || key == GLFW_KEY_W) {
						selectedItem = flooredModulo(selectedItem - 1, static_cast<int>(levelMenuConf.size()));
						menuDrawn = false;
					}
					if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) {
						selectedItem = flooredModulo(selectedItem + 1, static_cast<int>(levelMenuConf.size()));
						menuDrawn = false;
					}
					if (key == GLFW_KEY_LEFT|| key == GLFW_KEY_A) {
						if (selectedItem - perPage >= 0) {
							selectedItem = selectedItem - perPage;
							menuDrawn = false;
						}
					}
					if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
						if (selectedItem + perPage < static_cast<int>(levelMenuConf.size())) {
							selectedItem = selectedItem + perPage;
							menuDrawn = false;
						} else if (page < static_cast<int>(levelMenuConf.size()) / perPage) {
							// Go to the last page if we are not on it.
							selectedItem = static_cast<int>(levelMenuConf.size()) - 1;
							menuDrawn = false;
						}
					}
					if (key == GLFW_KEY_ESCAPE) {
						return SceneAborted_tag{};
					}
				}
			}
		}
	}
}
