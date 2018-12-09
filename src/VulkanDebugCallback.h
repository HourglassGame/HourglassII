#ifndef HG_VULKANDEBUGCALLBACK_H
#define HG_VULKANDEBUGCALLBACK_H
#include "GlobalConst.h"
#include <vulkan/vulkan.h>
namespace hg {

    static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << "\n";;

        return VK_FALSE;
    }
    class VulkanDebugCallback final {
    public:
        VulkanDebugCallback(
            VkInstance const &i
        )
            : i(i)
            , vkCreateDebugUtilsMessengerEXT(reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(i, "vkCreateDebugUtilsMessengerEXT")))
            , vkDestroyDebugUtilsMessengerEXT(reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(i, "vkDestroyDebugUtilsMessengerEXT")))
        {
            if (!enableValidationLayers) return;

            if (!vkCreateDebugUtilsMessengerEXT || !vkDestroyDebugUtilsMessengerEXT) {
                throw std::exception("Couldn't load vkCreateDebugUtilsMessengerEXT or vkDestroyDebugUtilsMessengerEXT functions");
            }

            VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = vulkanDebugCallback;

            if (vkCreateDebugUtilsMessengerEXT(i, &createInfo, nullptr, &callback) != VK_SUCCESS) {
                throw std::runtime_error("Couldn't enable debug callback");
            }
        }
        VulkanDebugCallback(VulkanDebugCallback const&) = delete;
        VulkanDebugCallback(VulkanDebugCallback &&) = delete;
        VulkanDebugCallback &operator=(VulkanDebugCallback const&) = delete;
        VulkanDebugCallback &operator=(VulkanDebugCallback &&) = delete;
        ~VulkanDebugCallback() noexcept {
            if (!enableValidationLayers) return;
            vkDestroyDebugUtilsMessengerEXT(i, callback, nullptr);
        }
        VkInstance const &i;
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
        VkDebugUtilsMessengerEXT callback;
    };
}
#endif // !HG_VULKANDEBUGCALLBACK_H
