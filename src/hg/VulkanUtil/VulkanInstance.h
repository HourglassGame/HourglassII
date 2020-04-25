#ifndef HG_VULKANINSTANCE_H
#define HG_VULKANINSTANCE_H

#include "VulkanExceptions.h"
#include <vulkan/vulkan.h>
namespace hg {

    class VulkanInstance final {

    public:
        VulkanInstance(
            VkInstanceCreateInfo const& createInfo
        )
        {
            {
                auto const res{vkCreateInstance(&createInfo, nullptr, &i)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "Couldn't create Vulkan instance"));
                }
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
