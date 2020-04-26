#include "VulkanEngine.h"
namespace hg {
namespace {
    static bool checkValidationLayerSupport() {
        uint32_t layerCount = 0;
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read vulkan layer count"));
            }
        }
        std::vector<VkLayerProperties> availableLayers(layerCount);
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Failed to read vulkan layers"));
            }
        }
        availableLayers.resize(layerCount);

        std::vector<const char*> validationLayersSortUnique(validationLayers);
        boost::erase(validationLayersSortUnique, boost::unique<boost::return_found_end>(boost::sort(validationLayersSortUnique, strcmporder), strcmpeq));

        std::vector<const char*> availableLayersSortUnique;
        boost::push_back(availableLayersSortUnique, availableLayers | boost::adaptors::transformed([](VkLayerProperties const &a) {return a.layerName; }));
        boost::erase(availableLayersSortUnique, boost::unique<boost::return_found_end>(boost::sort(availableLayersSortUnique, strcmporder), strcmpeq));

        return boost::range::includes(
            availableLayersSortUnique,
            validationLayersSortUnique,
            strcmporder);
    }

    static std::vector<const char*> getRequiredExtensions() {
        if (!glfwVulkanSupported()) {
            BOOST_THROW_EXCEPTION(std::exception("Vulkan support insufficient to run in GLFW"));
        }
        uint32_t glfwExtensionCount = 0;
        const char ** const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (!glfwExtensions) {
            BOOST_THROW_EXCEPTION(std::exception("Couldn't load GLFW vulkan extension list (glfwGetRequiredInstanceExtensions)"));
        }
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VkInstanceCreateInfo makeInstanceCreateInfo(VkApplicationInfo const * const pApplicationInfo, std::vector<char const *> const &extensions) {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            BOOST_THROW_EXCEPTION(std::exception("validation layer(s) requested by not supported"));
        }

        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.pApplicationInfo = pApplicationInfo;

        if (enableValidationLayers) {
            info.enabledLayerCount = gsl::narrow<uint32_t>(validationLayers.size());
            info.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            info.enabledLayerCount = 0;
            info.ppEnabledLayerNames = nullptr;
        }

        if (!glfwVulkanSupported()) {
            BOOST_THROW_EXCEPTION(std::exception("Vulkan support insufficient to run in GLFW"));
        }

        info.enabledExtensionCount = gsl::narrow<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
        return info;
    }

    static std::vector<VkImage> createSwapChainImages(
        VkDevice const device,
        VkSwapchainKHR const swapChain
    )
    {
        uint32_t imageCount{};
        std::vector<VkImage> swapChainImages;
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read swapchain image count"));
            }
        }
        swapChainImages.resize(imageCount);
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read swapchain images"));
            }
        }
        swapChainImages.resize(imageCount);
        return swapChainImages;
    }
    static std::vector<VulkanImageView> createSwapChainImageViews(
        VkDevice const device, VkFormat const imageFormat, std::vector<VkImage> const &swapChainImages) {
        std::vector<VulkanImageView> swapChainImageViews;
        auto rng{ swapChainImages | boost::adaptors::transformed([device, imageFormat](auto const &image) {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = image;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            return VulkanImageView{device, createInfo};
        }) };
        swapChainImageViews.assign(std::make_move_iterator(std::begin(rng)), std::make_move_iterator(std::end(rng)));

        return swapChainImageViews;
    }
    static std::vector<VulkanFramebufferHG> createSwapchainFramebuffers(
        VkDevice const device,
        VkRenderPass const renderPass,
        VkExtent2D const swapChainExtent,
        std::vector<VulkanImageView> const &swapChainImageViews)
    {
        std::vector<VulkanFramebufferHG> framebuffers;
        framebuffers.reserve(swapChainImageViews.size());
        for (auto const &swapChainImageView : swapChainImageViews) {
            framebuffers.emplace_back(device, swapChainImageView.imageView, renderPass, swapChainExtent);
        }
        return framebuffers;
    }
    static std::vector<VulkanSemaphoreHG> createImageAvailableSemaphores(VkDevice const device) {
        std::vector<VulkanSemaphoreHG> s;
        for (auto i{0}; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            s.emplace_back(device);
        }
        return s;
    }
    static std::vector<VulkanSemaphoreHG> createRenderFinishedSemaphores(VkDevice const device) {
        std::vector<VulkanSemaphoreHG> s;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            s.emplace_back(device);
        }
        return s;
    }
    static std::vector<VulkanFenceHG> createInFlightFences(VkDevice const device) {
        std::vector<VulkanFenceHG> f;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            f.emplace_back(device);
        }
        return f;
    }
    static VkExtent2D getFramebufferSize(GLFWwindow &w) {
        int width, height;
        glfwGetFramebufferSize(&w, &width, &height);
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }
}//unnamed namespace







void VulkanEngine::framebufferResizeCallback(GLFWwindow* const window, int const width, int const height) {
    auto const eng{static_cast<VulkanEngine*>(glfwGetWindowUserPointer(window))};
    decltype(frameBufferSizeMutex)::scoped_lock lock(eng->frameBufferSizeMutex);
    eng->newFramebufferSize = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    //TODO: There may be a race condition here between the time this lock is released and when the control returns to the event loop.
    //Need to add some additional locks to make sure the renderer doesn't run during this period of time.
    //(My understanding is that the framebuffer gets resized once the control has returned to the event loop;
    // so ideally we would wait until the next event is processed by the loop, to be sure that this processing has completed)


    //https://github.com/KhronosGroup/Vulkan-Docs/issues/1144
    //https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1340
    //https://github.com/KhronosGroup/Vulkan-Docs/issues/388
}

VulkanEngine::VulkanEngine(
    GLFWwindow &w
)
    : w(&w)
    , instance(makeInstanceCreateInfo(nullptr, getRequiredExtensions()))
    , oldFramebufferSize(getFramebufferSize(w))
    , newFramebufferSize(getFramebufferSize(w))
    , debugCallback(instance.i)
    , surface(instance.i, &w)
    , physicalDevice(
        [&]{
            auto const possiblePhysicalDevices{enumerateSuitablePhysicalDevices(instance.i, surface.surface)};
            if (possiblePhysicalDevices.empty()) {
                BOOST_THROW_EXCEPTION(std::exception("Failed to find GPU supporting features required for Hourglass II"));
            }
            return possiblePhysicalDevices.front();
        }())
    , device(physicalDevice, surface.surface)
    //TODO: if frameBufferWidth or Height is 0, don't create the swapChain; rather than
    //making an invalid swapChain!
    //Attempting to make a swapChain with width or height 0 violates the vulkan spec!
    , swapChain(physicalDevice, device.h(), surface.surface, oldFramebufferSize, VK_NULL_HANDLE)
    , swapChainImages(createSwapChainImages(device.h(), swapChain.h()))
    , swapChainImageViews(createSwapChainImageViews(device.h(), physicalDevice.surfaceFormat.format, swapChainImages))
    , renderPass(device.h(), physicalDevice.surfaceFormat.format)
    , swapChainFramebuffers(createSwapchainFramebuffers(device.h(), renderPass.h(), swapChain.extent(), swapChainImageViews))
    , imageAvailableSemaphores(createImageAvailableSemaphores(device.h()))
    , renderFinishedSemaphores(createRenderFinishedSemaphores(device.h()))
    , inFlightFences(createInFlightFences(device.h()))
    , currentFrame(0)
{
    glfwSetWindowUserPointer(&w, this);
    glfwSetFramebufferSizeCallback(&w, framebufferResizeCallback);
    //glfwSetWindowRefreshCallback(&w, windowRefreshCallback); //TODO
}

void VulkanEngine::recreateSwapChain(VulkanRenderer &renderer) {
    vkDeviceWaitIdle(device.h());
    framebufferResizedCheck();

    auto const maxImageExtent{querySwapChainMaxImageExtent(physicalDevice.physicalDevice, surface.surface)};

    if (oldFramebufferSize.width == 0 || oldFramebufferSize.height == 0
        || maxImageExtent.width == 0 || maxImageExtent.height == 0) {
        //Can't render into 0-size swapchain.
        return;
    }
    VulkanSwapChainHG newSwapChain(physicalDevice, device.h(), surface.surface, oldFramebufferSize, swapChain.h());
    std::vector<VkImage> newSwapChainImages(createSwapChainImages(device.h(), newSwapChain.h()));
    std::vector<VulkanImageView> newSwapChainImageViews(createSwapChainImageViews(device.h(), physicalDevice.surfaceFormat.format, newSwapChainImages));
    std::vector<VulkanFramebufferHG> newSwapChainFramebuffers(createSwapchainFramebuffers(device.h(), renderPass.h(), newSwapChain.extent(), newSwapChainImageViews));

    renderer.updateSwapChainData(renderPass.h(), newSwapChain.extent());

    swapChain = std::move(newSwapChain);
    swapChainImages = std::move(swapChainImages);
    swapChainImageViews = std::move(newSwapChainImageViews);
    swapChainFramebuffers = std::move(newSwapChainFramebuffers);
}

bool VulkanEngine::framebufferResizedCheck() {
    if (oldFramebufferSize.width != newFramebufferSize.width || oldFramebufferSize.height != newFramebufferSize.height) {
        oldFramebufferSize = newFramebufferSize;
        return true;
    }
    return false;
}


void VulkanEngine::drawFrame(VulkanRenderer &renderer) {
    decltype(frameBufferSizeMutex)::scoped_lock lock(frameBufferSizeMutex);
    {
        auto const maxImageExtent{querySwapChainMaxImageExtent(physicalDevice.physicalDevice, surface.surface)};
        //Can't render into 0-size surface
        if (maxImageExtent.width == 0 || maxImageExtent.height == 0) return;
    }
    {
        auto const fences{std::array{inFlightFences[currentFrame].h()}};
        auto const res{vkWaitForFences(device.h(), fences.size(), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max())};
        if (res != VK_SUCCESS) {
            BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't wait for fence"));
        }
    }
    uint32_t imageIndex{};
    while (true) {
        auto const res{ vkAcquireNextImageKHR(device.h(), swapChain.h(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame].h(), VK_NULL_HANDLE, &imageIndex)};
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain(renderer);
            continue;
        }
        else if (!(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)) {
            BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't Acquire next Image"));
        }
        break;
    }
    auto const renderedCommandBuffers{renderer.renderFrame(currentFrame, swapChainFramebuffers[imageIndex].h())};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].h() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = gsl::narrow<uint32_t>(renderedCommandBuffers.size());
    submitInfo.pCommandBuffers = renderedCommandBuffers.data();

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].h() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    {
        auto const fences{std::array{inFlightFences[currentFrame].h()}};
        auto const res{vkResetFences(device.h(), fences.size(), fences.data())};
        if (res != VK_SUCCESS) {
            BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't reset fence"));
        }
    }
    {
        auto const res{vkQueueSubmit(device.graphicsQ(), 1, &submitInfo, inFlightFences[currentFrame].h())};
        if (res != VK_SUCCESS) {
            BOOST_THROW_EXCEPTION(std::system_error(res, "failed to submit draw command buffer!"));
        }
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    auto const swapChains{std::array{swapChain.h()}};
    presentInfo.swapchainCount = gsl::narrow<uint32_t>(swapChains.size());
    presentInfo.pSwapchains = swapChains.data();

    presentInfo.pImageIndices = &imageIndex;

    {
        auto const res{vkQueuePresentKHR(device.presentQ(), &presentInfo)};
        if (res == VK_ERROR_OUT_OF_DATE_KHR || framebufferResizedCheck()) {
            recreateSwapChain(renderer);
        }
        else if (!(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)) {
            BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't present queue"));
        }
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void VulkanEngine::idle(VulkanRenderer &vkRenderer) {
    for(int f{0}; f != MAX_FRAMES_IN_FLIGHT; ++f) {
        {
            auto const res{vkGetFenceStatus(device.h(), inFlightFences[f].h())};
            if (res == VK_SUCCESS) {
                vkRenderer.frameEnded(f);
            }
            else if (!(res == VK_SUCCESS || res == VK_NOT_READY)) {
                BOOST_THROW_EXCEPTION(std::system_error(res, "Idle getFenceStatus failed"));
            }
        }
    }
}

}//namespace hg
