#ifndef HG_VULKANUTIL_H
#define HG_VULKANUTIL_H
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
namespace hg {
    std::vector<const char*> const deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily && presentFamily;
        }
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR const surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        queueFamilies.resize(queueFamilyCount);
        uint32_t i{0};
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }
            }
            if (indices.isComplete()) break;
            ++i;
        }

        return indices;
    }

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice const device, VkSurfaceKHR const surface) {
        SwapChainSupportDetails details;

        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities) != VK_SUCCESS) {
            throw std::exception("Couldn't Read Surface Capabilities");
        }

        uint32_t formatCount{};
        {
            auto const res{vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr)};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read surface formats count");
            }
        }

        details.formats.resize(formatCount);
        {
            auto const res{ vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data())};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read surface formats");
            }
        }
        details.formats.resize(formatCount);

        uint32_t presentModeCount{};
        {
            auto const res{vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr)};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read presentModes count");
            }
        }
        
        details.presentModes.resize(presentModeCount);
        {
            auto const res{ vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data())};
            if (!(res == VK_SUCCESS || VK_INCOMPLETE)) {
                throw std::exception("Couldn't read presentModes");
            }
        }
        details.presentModes.resize(presentModeCount);

        return details;
    }
}
#endif // !HG_VULKANUTIL_H
