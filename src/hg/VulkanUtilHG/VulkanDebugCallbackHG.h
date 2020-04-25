#ifndef HG_VULKANDEBUGCALLBACKHG_H
#define HG_VULKANDEBUGCALLBACKHG_H
#include "hg/VulkanUtil/VulkanDebugCallback.h"
#include "hg/GlobalConst.h"
#include <vulkan/vulkan.h>
#include <iostream>
namespace hg {
    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if (
            (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
         || (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT))
        {
            std::cerr << "validation error: " << pCallbackData->pMessage << "\n";
        }
        else if (
            (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
         || (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
        {
            std::cerr << "validation warning: " << pCallbackData->pMessage << "\n";
        }
        else {
            std::cerr << "validation info: " << pCallbackData->pMessage << "\n";
        }
        return VK_FALSE;
    }

    class VulkanDebugCallbackHG final {
    public:
        VulkanDebugCallbackHG(
            VkInstance const instance
        ) : debugCallback()
        {
            if (!enableValidationLayers) return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = vulkanDebugCallback;

            debugCallback.emplace(instance, createInfo);
        }
        VulkanDebugCallbackHG(VulkanDebugCallbackHG const&) = delete;
        VulkanDebugCallbackHG(VulkanDebugCallbackHG &&) noexcept = delete;
        VulkanDebugCallbackHG &operator=(VulkanDebugCallbackHG const&) = delete;
        VulkanDebugCallbackHG &operator=(VulkanDebugCallbackHG &&) noexcept = delete;
    private:
        std::optional<VulkanDebugCallback> debugCallback;
    };
}
#endif // !HG_VULKANDEBUGCALLBACKHG_H
