#ifndef HG_VULKANSWAPCHAIN_H
#define HG_VULKANSWAPCHAIN_H
#include "GlobalConst.h"
#include "VulkanUtil.h"
#include "Maths.h"
#include <vulkan/vulkan.h>
namespace hg {
    inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> const &availableFormats) {
        if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
            return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        }

        for (auto const &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }

            /*
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }*/
        }

        return availableFormats[0];
    }

    inline VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> const &availablePresentModes) {
        VkPresentModeKHR bestMode{VK_PRESENT_MODE_FIFO_KHR};

        for (auto const &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = availablePresentMode;
            }
        }

        return bestMode;
    }

    inline VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR const &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            //TODO: Dynamically change window width/height here!
            return {
                std::clamp<uint32_t>(WINDOW_DEFAULT_X, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                std::clamp<uint32_t>(WINDOW_DEFAULT_Y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }
    }

    class VulkanSwapChain final {
    public:
        VulkanSwapChain(
            VkPhysicalDevice const physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface
        )
          : device(device)
        {
            SwapChainSupportDetails const swapChainSupport{querySwapChainSupport(physicalDevice, surface)};

            surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR const presentMode{chooseSwapPresentMode(swapChainSupport.presentModes)};
            extent = chooseSwapExtent(swapChainSupport.capabilities);

            imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices const indices{findQueueFamilies(physicalDevice, surface)};
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                throw std::exception("failed to create swap chain!");
            }
        }
        VulkanSwapChain(VulkanSwapChain const&) = delete;
        VulkanSwapChain(VulkanSwapChain &&) = delete;
        VulkanSwapChain &operator=(VulkanSwapChain const&) = delete;
        VulkanSwapChain &operator=(VulkanSwapChain &&) = delete;
        ~VulkanSwapChain() noexcept {
            vkDestroySwapchainKHR(device, swapChain, nullptr);
        }
        uint32_t imageCount;
        VkSurfaceFormatKHR surfaceFormat;
        VkExtent2D extent;
        VkDevice device;
        VkSwapchainKHR swapChain;
    };
}
#endif // !HG_VULKANSWAPCHAIN_H
