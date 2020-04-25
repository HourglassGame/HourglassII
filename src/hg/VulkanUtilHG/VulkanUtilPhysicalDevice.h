#ifndef HG_VULKANUTILPHYSICALDEVICE_H
#define HG_VULKANUTILPHYSICALDEVICE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
namespace hg {
    //Vulkan Extensions required for HourglassII
    inline std::array const deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct QueueFamiliesForUses final {
        uint32_t graphicsFamily;//Index of queue family with VK_QUEUE_GRAPHICS_BIT set; to use for Graphics processing.
        uint32_t presentFamily; //Index of queue family with vkGetPhysicalDeviceSurfaceSupportKHR giving VK_TRUE result; to use for Present processing.
    };

    struct SwapChainSupportDetails final {
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct PossiblePhysicalDevice final {
        VkPhysicalDevice physicalDevice;
        QueueFamiliesForUses queueIndices;
        SwapChainSupportDetails swapChainSupport;
        //TODO: Potentially also list supported extensions, in the future where we optionally
        //use some extensions.

        //Score for default ranking.
        //By default, the highest score will be chosen. In the future we may allow the user to override the default.
        //Score = 0 indicates the device is not suitable.
        uint32_t score;
    };

    std::vector<PossiblePhysicalDevice> enumerateSuitablePhysicalDevices(VkInstance const instance, VkSurfaceKHR const surface);

}

#endif //!HG_VULKANUTILPHYSICALDEVICE_H
