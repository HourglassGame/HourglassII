#ifndef HG_VULKANENGINE_H
#define HG_VULKANENGINE_H
#include "VulkanSwapChain.h"
#include "VulkanSwapChainImageView.h"
#include "VulkanSurface.h"
#include "VulkanRenderPass.h"
#include "VulkanInstance.h"
#include "VulkanDebugCallback.h"
#include "VulkanUtil.h"
#include "VulkanLogicalDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanMemory.h"
#include "VulkanBuffer.h"
#include <GLFW/glfw3.h>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <vector>
#include <optional>
#include "GlobalConst.h"
namespace hg {
    inline int const MAX_FRAMES_IN_FLIGHT = 2;
    inline auto const strcmporder{ [](char const * const a, char const * const b) {return strcmp(a, b) < 0; } };

    inline bool checkValidationLayerSupport() {
        uint32_t layerCount = 0;
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read vulkan layer count", res);
            }
        }
        std::vector<VkLayerProperties> availableLayers(layerCount);
        {
            auto const res{ vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read vulkan layers", res);
            }
        }
        availableLayers.resize(layerCount);

        std::vector<const char*> validationLayersSortUnique(validationLayers);
        boost::erase(validationLayersSortUnique, boost::unique(boost::sort(validationLayersSortUnique, strcmporder), strcmporder));

        std::vector<const char*> availableLayersSortUnique;
        boost::push_back(availableLayersSortUnique, availableLayers | boost::adaptors::transformed([](VkLayerProperties const &a) {return a.layerName; }));

        boost::erase(availableLayersSortUnique, boost::unique(boost::sort(availableLayersSortUnique, strcmporder), strcmporder));

        return boost::range::includes(
            availableLayersSortUnique,
            validationLayersSortUnique,
            strcmporder);
    }

    std::vector<const char*> getRequiredExtensions() {
        if (!glfwVulkanSupported()) {
            throw std::exception("Vulkan support insufficient to run in GLFW");
        }
        uint32_t glfwExtensionCount = 0;
        const char ** const glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (!glfwExtensions) {
            throw std::exception("Couldn't load GLFW vulkan extension list (glfwGetRequiredInstanceExtensions)");
        }
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    inline VkInstanceCreateInfo makeInstanceCreateInfo(VkApplicationInfo const * const pApplicationInfo, std::vector<char const *> const &extensions) {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::exception("validation layer(s) requested by not supported");
        }

        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.pApplicationInfo = pApplicationInfo;

        if (enableValidationLayers) {
            info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            info.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            info.enabledLayerCount = 0;
            info.ppEnabledLayerNames = nullptr;
        }

        if (!glfwVulkanSupported()) {
            throw std::exception("Vulkan support insufficient to run in GLFW");
        }

        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();
        return info;
    }
    inline bool checkDeviceExtensionSupport(VkPhysicalDevice const device) {
        uint32_t extensionCount{0};
        {
            auto const res{vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr)};
            if (!(res== VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't count Device Extensions vkEnumerateDeviceExtensionProperties");
            }
        }
        
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        {
            auto const res{vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data())};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't read Device Extensions vkEnumerateDeviceExtensionProperties");
            }
            availableExtensions.resize(extensionCount);
        }

        std::vector<const char*> deviceExtensionsSortUnique(deviceExtensions);
        boost::erase(deviceExtensionsSortUnique, boost::unique(boost::sort(deviceExtensionsSortUnique, strcmporder), strcmporder));

        std::vector<const char*> availableExtensionsSortUnique;
        boost::push_back(availableExtensionsSortUnique, availableExtensions | boost::adaptors::transformed([](VkExtensionProperties const &a) {return a.extensionName; }));
        boost::erase(availableExtensionsSortUnique, boost::unique(boost::sort(availableExtensionsSortUnique, strcmporder), strcmporder));

        return boost::range::includes(
            availableExtensionsSortUnique,
            deviceExtensionsSortUnique,
            strcmporder);
    }

    inline bool checkSwapChainSupport(SwapChainSupportDetails const &swapChainSupport) {
        return !(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty());
    }

    inline bool isDeviceSuitable(VkPhysicalDevice const device, VkSurfaceKHR const surface) {
        return findQueueFamilies(device, surface).isComplete()
            && checkDeviceExtensionSupport(device)
            && checkSwapChainSupport(querySwapChainSupport(device, surface));
    }

    inline VkPhysicalDevice pickPhysicalDevice(VkInstance const &instance, VkSurfaceKHR const surface) {
        uint32_t deviceCount = 0;
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Failed to read physical device count", res);
            }
        }
        if (deviceCount == 0) {
            throw std::exception("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        {
            auto const res{vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data())};
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("failed to read physical devices", res);
            }
        }
        devices.resize(deviceCount);
        auto const suitableDeviceIt{boost::find_if(devices, [&surface](VkPhysicalDevice device) {return isDeviceSuitable(device, surface);})};
        if (suitableDeviceIt == std::end(devices)) {
            throw std::exception("failed to find a suitable GPU!");
        }
        return *suitableDeviceIt;
    }
    inline std::vector<VkImage> createSwapChainImages(
        VkDevice const device,
        VkSwapchainKHR const swapChain,
        uint32_t &imageCount)
    {
        std::vector<VkImage> swapChainImages;
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't read swapchain image count");
            }
        }
        swapChainImages.resize(imageCount);
        {
            auto const res{ vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()) };
            if (!(res == VK_SUCCESS || res == VK_INCOMPLETE)) {
                throw std::exception("Couldn't read swapchain images");
            }
        }
        swapChainImages.resize(imageCount);
        return swapChainImages;
    }
    inline std::vector<VulkanSwapChainImageView> createSwapChainImageViews(
        VkDevice const device, VkFormat const imageFormat, std::vector<VkImage> const &swapChainImages) {
        std::vector<VulkanSwapChainImageView> swapChainImageViews;
        auto rng{ swapChainImages | boost::adaptors::transformed([device, imageFormat](auto const &i) {return VulkanSwapChainImageView{device, imageFormat, i}; }) };
        swapChainImageViews.assign(std::make_move_iterator(std::begin(rng)), std::make_move_iterator(std::end(rng)));

        return swapChainImageViews;
    }
    inline std::vector<VulkanFramebuffer> createSwapchainFramebuffers(
        VkDevice const device,
        VkRenderPass const renderPass,
        VkExtent2D const swapChainExtent,
        std::vector<VulkanSwapChainImageView> const &swapChainImageViews)
    {
        std::vector<VulkanFramebuffer> framebuffers;
        framebuffers.reserve(swapChainImageViews.size());
        for (auto const &swapChainImageView : swapChainImageViews) {
            framebuffers.emplace_back(device, swapChainImageView.imageView, renderPass, swapChainExtent);
        }
        return framebuffers;
    }
    /*
    inline std::vector<VkCommandBuffer> createCommandBuffers(
        std::vector<VulkanFramebuffer> const &swapChainFramebuffers,
        VkCommandPool const commandPool,
        VkDevice const device,
        VkRenderPass const renderPass,
        VkExtent2D const swapChainExtent,
        VkPipeline const graphicsPipeline
    )
    {
        std::vector<VkCommandBuffer> commandBuffers(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::exception("failed to allocate command buffers!");
        }

        for (std::size_t i{0}, end{commandBuffers.size()}; i < end; ++i) {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::exception("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i].framebuffer;
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapChainExtent;

            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::exception("failed to record command buffer!");
            }
        }
        return commandBuffers;
    }
    */
    inline std::vector<VkCommandBuffer> createCommandBuffers2(
        VkCommandPool const commandPool,
        VkDevice const device
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
    inline std::vector<VulkanSemaphore> createImageAvailableSemaphores(VkDevice const device) {
        std::vector<VulkanSemaphore> s;
        for (auto i{0}; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            s.emplace_back(device);
        }
        return s;
    }
    inline std::vector<VulkanSemaphore> createRenderFinishedSemaphores(VkDevice const device) {
        std::vector<VulkanSemaphore> s;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            s.emplace_back(device);
        }
        return s;
    }
    inline std::vector<VulkanFence> createInFlightFences(VkDevice const device) {
        std::vector<VulkanFence> f;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            f.emplace_back(device);
        }
        return f;
    }
    inline std::vector<VulkanBuffer> createVertexBuffers(VkDevice const device) {
        std::vector<VulkanBuffer> b;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            b.emplace_back(device);
        }
        return b;
    }
    inline std::vector<VulkanMemory> createVertexMemories(VkDevice const device) {
        std::vector<VulkanMemory> m;
        for (auto i{ 0 }; i != MAX_FRAMES_IN_FLIGHT; ++i) {
            m.emplace_back(device);
        }
        return m;
    }
    inline uint32_t findMemoryType(VkPhysicalDevice const physicalDevice, uint32_t const typeFilter, VkMemoryPropertyFlags const properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::exception("failed to find suitable memory type!");
    }
    class VulkanEngine final {
    public:
        VulkanEngine(
            GLFWwindow &w
        ) : w(&w)
          , instance(makeInstanceCreateInfo(nullptr, getRequiredExtensions()))
          , debugCallback(instance.i)
          , surface(instance.i, &w)
          , physicalDevice(pickPhysicalDevice(instance.i, surface.surface))
          , logicalDevice(physicalDevice, surface.surface)
          , swapChain(physicalDevice, logicalDevice.device, surface.surface)
          , swapChainImages(createSwapChainImages(logicalDevice.device, swapChain.swapChain, swapChain.imageCount))
          , swapChainImageViews(createSwapChainImageViews(logicalDevice.device, swapChain.surfaceFormat.format, swapChainImages))
          , renderPass(logicalDevice.device, swapChain.surfaceFormat.format)
          , pipelineLayout(logicalDevice.device, swapChain.extent)
          , graphicsPipeline(logicalDevice.device, swapChain.extent, pipelineLayout.pipelineLayout, renderPass.renderPass)
          , swapChainFramebuffers(createSwapchainFramebuffers(logicalDevice.device, renderPass.renderPass, swapChain.extent, swapChainImageViews))
          , commandPool(logicalDevice.device, physicalDevice, surface.surface)
          //, commandBuffers(createCommandBuffers(swapChainFramebuffers, commandPool.commandPool, logicalDevice.device, renderPass.renderPass, swapChain.extent, graphicsPipeline.graphicsPipeline))
          , commandBuffers2(createCommandBuffers2(commandPool.commandPool, logicalDevice.device))
          , vertexBufferMemories(createVertexMemories(logicalDevice.device))
          , vertexBuffers(createVertexBuffers(logicalDevice.device))
          , imageAvailableSemaphores(createImageAvailableSemaphores(logicalDevice.device))
          , renderFinishedSemaphores(createRenderFinishedSemaphores(logicalDevice.device))
          , inFlightFences(createInFlightFences(logicalDevice.device))
          , currentFrame(0)
        {
        }
        void drawFrame(std::vector<vec2<float>> const &vertices = std::vector<vec2<float>>{{0.f,-.5f},{0.5f,0.5f},{-0.5f,0.5f},{-1.f,0.f},{-1.f,-1.f},{0.f,-1.f} }) {
            if (vkWaitForFences(logicalDevice.device, 1, &inFlightFences[currentFrame].fence, VK_TRUE, std::numeric_limits<uint64_t>::max()) != VK_SUCCESS) {
                throw std::exception("Couldn't wait for fence");
            }
            if (vkResetFences(logicalDevice.device, 1, &inFlightFences[currentFrame].fence) != VK_SUCCESS) {
                throw std::exception("Couldn't reset fence");
            }

            uint32_t imageIndex;
            if (vkAcquireNextImageKHR(logicalDevice.device, swapChain.swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame].semaphore, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) {
                throw std::exception("Couldn't Acquire next Image");
            }

            auto &vertexBuffer{ vertexBuffers[currentFrame] };
            vertexBuffer = VulkanBuffer(logicalDevice.device);
            {
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = sizeof(vertices[0]) * vertices.size();
                bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                vertexBuffer = VulkanBuffer(logicalDevice.device, bufferInfo);
            }
            auto &vertexBufferMemory{vertexBufferMemories[currentFrame]};
            vertexBufferMemory = VulkanMemory(logicalDevice.device);
            {
                VkMemoryRequirements memRequirements;
                vkGetBufferMemoryRequirements(logicalDevice.device, vertexBuffer.buffer, &memRequirements);

                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vertexBufferMemory = VulkanMemory(logicalDevice.device, allocInfo);
            }
            if (vkBindBufferMemory(logicalDevice.device, vertexBuffer.buffer, vertexBufferMemory.memory, 0) != VK_SUCCESS) {
                throw std::exception("Couldn't bind buffer memory");
            }

            void* data;
            if (vkMapMemory(logicalDevice.device, vertexBufferMemory.memory, 0, sizeof(vertices[0]) * vertices.size(), 0, &data) != VK_SUCCESS){
                throw std::exception("Couldn't map memory for vertex buffer");
            }
            memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
            vkUnmapMemory(logicalDevice.device, vertexBufferMemory.memory);

            auto const &commandBuffer{commandBuffers2[currentFrame]};
            {
                if (vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
                    throw std::exception("Couldn't reset command buffer!");
                }

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

                if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::exception("failed to begin recording command buffer!");
                }

                VkRenderPassBeginInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass.renderPass;
                renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex].framebuffer;
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = swapChain.extent;

                VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
                renderPassInfo.clearValueCount = 1;
                renderPassInfo.pClearValues = &clearColor;

                vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);
                
                VkBuffer vertexBufferArr[] = { vertexBuffers[currentFrame].buffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBufferArr, offsets);

                vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

                vkCmdEndRenderPass(commandBuffer);

                if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                    throw std::exception("failed to record command buffer!");
                }
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame].semaphore };
            VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame].semaphore };
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame].fence) != VK_SUCCESS) {
                throw std::exception("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { swapChain.swapChain };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;

            presentInfo.pImageIndices = &imageIndex;

            if (vkQueuePresentKHR(logicalDevice.presentQueue, &presentInfo) != VK_SUCCESS) {
                throw std::exception("Couldn't present queue");
            }

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
        VulkanEngine(VulkanEngine const&) = delete;
        VulkanEngine(VulkanEngine &&) = delete;
        VulkanEngine &operator=(VulkanEngine const&) = delete;
        VulkanEngine &operator=(VulkanEngine &&) = delete;
        ~VulkanEngine() noexcept {
            vkDeviceWaitIdle(logicalDevice.device); //Something like this is needed, but maybe not this exact implementation
        }
    private:
        GLFWwindow *w;
        VulkanInstance instance;
        VulkanDebugCallback debugCallback;
        VulkanSurface surface;
        VkPhysicalDevice physicalDevice;
        VulkanLogicalDevice logicalDevice;
        VulkanSwapChain swapChain;
        std::vector<VkImage> swapChainImages;
        std::vector<VulkanSwapChainImageView> swapChainImageViews;
        VulkanRenderPass renderPass;
        VulkanPipelineLayout pipelineLayout;
        VulkanGraphicsPipeline graphicsPipeline;
        std::vector<VulkanFramebuffer> swapChainFramebuffers;
        VulkanCommandPool commandPool;
        //std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkCommandBuffer> commandBuffers2;
        std::vector<VulkanMemory> vertexBufferMemories;
        std::vector<VulkanBuffer> vertexBuffers;
        std::vector<VulkanSemaphore> imageAvailableSemaphores;
        std::vector<VulkanSemaphore> renderFinishedSemaphores;
        std::vector<VulkanFence> inFlightFences;
        size_t currentFrame;
    };
}
#endif // !HG_VULKANENGINE_H
