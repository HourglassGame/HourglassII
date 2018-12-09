#ifndef HG_VULKANINSTANCE_H
#define HG_VULKANINSTANCE_H

#include <vulkan/vulkan.h>
namespace hg {

    class VulkanInstance final {

    public:
        VulkanInstance(
            VkInstanceCreateInfo const& createInfo
        )
        {
            if(vkCreateInstance(&createInfo, nullptr, &i) != VK_SUCCESS) {
                throw std::exception("Couldn't create Vulkan instance");
            }
        }
        VulkanInstance(VulkanInstance const&) = delete;
        VulkanInstance(VulkanInstance &&) = delete;
        VulkanInstance &operator=(VulkanInstance const&) = delete;
        VulkanInstance &operator=(VulkanInstance &&) = delete;
        ~VulkanInstance() noexcept {
            vkDestroyInstance(i, nullptr);
        }
        VkInstance i;
    };
}
#endif // !HG_VULKANINSTANCE_H
