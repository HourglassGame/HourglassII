#ifndef HG_VULKANSWAPCHAIN_H
#define HG_VULKANSWAPCHAIN_H
#include "VulkanUtil.h"

#include "VulkanExceptions.h"
#include "hg/GlobalConst.h"

#include "hg/VulkanUtilHG/VulkanUtilPhysicalDevice.h"

#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
#include <utility>
#include <vector>

namespace hg {

    class VulkanSwapChain final {
    public:
        explicit VulkanSwapChain(VkDevice const device)
            : device(device)
            , swapChain(VK_NULL_HANDLE)
            , imageCount()
            , extent()
        {}
        explicit VulkanSwapChain(
            PossiblePhysicalDevice const &physicalDevice,
            VkDevice const device,
            VkSurfaceKHR const surface,
            VkExtent2D const glfwFramebufferExtent,
            VkSwapchainKHR oldSwapchain
        )
          : device(device)
        {
            VkSurfaceCapabilitiesKHR capabilities{};
            {
                auto const res{vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.physicalDevice, surface, &capabilities)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't read surface capabilities"));
                }
            }

            extent = [](VkExtent2D const glfwFramebufferExtent, VkSurfaceCapabilitiesKHR const &capabilities) -> VkExtent2D {
                if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                    return capabilities.currentExtent;
                }
                else {
                    return {
                        std::clamp<uint32_t>(glfwFramebufferExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
                        std::clamp<uint32_t>(glfwFramebufferExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
                    };
                }
            }(glfwFramebufferExtent, capabilities);

            imageCount = capabilities.minImageCount + 1;
            if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
                imageCount = capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = physicalDevice.surfaceFormat.format;
            createInfo.imageColorSpace = physicalDevice.surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            //QueueFamilyIndices const indices{findQueueFamilies(physicalDevice, surface)};
            auto queueFamilyIndices{std::array{ physicalDevice.queueIndices.graphicsFamily, physicalDevice.queueIndices.presentFamily }};

            if (physicalDevice.queueIndices.graphicsFamily != physicalDevice.queueIndices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = physicalDevice.presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = oldSwapchain;
            {
                auto const res{vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create swap chain!"));
                }
            }
        }
        VulkanSwapChain(VulkanSwapChain const&) = delete;
        VulkanSwapChain(VulkanSwapChain &&o) noexcept
            : device(o.device)
            , swapChain(std::exchange(o.swapChain, VkSwapchainKHR{VK_NULL_HANDLE}))
            , imageCount(o.imageCount)
            , extent(o.extent)
        {
        }
        VulkanSwapChain &operator=(VulkanSwapChain const&) = delete;
        VulkanSwapChain &operator=(VulkanSwapChain &&o) noexcept {
            std::swap(device, o.device);
            std::swap(swapChain, o.swapChain);
            std::swap(imageCount, o.imageCount);
            std::swap(extent, o.extent);
            return *this;
        }
        ~VulkanSwapChain() noexcept {
            vkDestroySwapchainKHR(device, swapChain, nullptr);
        }
        VkDevice device;
        VkSwapchainKHR swapChain;
        uint32_t imageCount;
        VkExtent2D extent;
    };
}
#endif // !HG_VULKANSWAPCHAIN_H
