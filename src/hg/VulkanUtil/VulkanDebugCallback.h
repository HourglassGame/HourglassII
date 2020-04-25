#ifndef HG_VULKANDEBUGCALLBACK_H
#define HG_VULKANDEBUGCALLBACK_H
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanDebugCallback final {
    public:
        explicit VulkanDebugCallback(
            VkInstance const instance
           ,VkDebugUtilsMessengerCreateInfoEXT const &createInfo
        )
            : instance(instance)
            , vkCreateDebugUtilsMessengerEXT(reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")))
            , vkDestroyDebugUtilsMessengerEXT(reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")))
            , callback([&]{
                if (!vkCreateDebugUtilsMessengerEXT || !vkDestroyDebugUtilsMessengerEXT) {
                    BOOST_THROW_EXCEPTION(std::exception("Couldn't load vkCreateDebugUtilsMessengerEXT or vkDestroyDebugUtilsMessengerEXT functions"));
                }
                VkDebugUtilsMessengerEXT cb{};
                {
                    auto const res{vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &cb)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't enable debug callback"));
                    }
                }
                return cb;
              }())
        {
        }
        VulkanDebugCallback(VulkanDebugCallback const&) = delete;
        VulkanDebugCallback(VulkanDebugCallback &&) noexcept = delete;
        VulkanDebugCallback &operator=(VulkanDebugCallback const&) = delete;
        VulkanDebugCallback &operator=(VulkanDebugCallback &&) noexcept = delete;
        ~VulkanDebugCallback() noexcept {
            vkDestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
        }
    private:
        VkInstance instance;
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
        VkDebugUtilsMessengerEXT callback;
    };
}
#endif // !HG_VULKANDEBUGCALLBACK_H
