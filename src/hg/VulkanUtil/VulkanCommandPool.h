#ifndef HG_VULKANCOMMANDPOOL_H
#define HG_VULKANCOMMANDPOOL_H
#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
namespace hg {
    class VulkanCommandPool final {
    public:
        explicit VulkanCommandPool(
            VkDevice const device,
            VkCommandPoolCreateInfo const &poolInfo
        ) : device(device)
          , commandPool([&]{
                VkCommandPool cp{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateCommandPool(device, &poolInfo, nullptr, &cp)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create command pool!"));
                    }
                }
                return cp;
            }())
        {
        }
        VulkanCommandPool(VulkanCommandPool const&) = delete;
        VulkanCommandPool(VulkanCommandPool &&o) = delete;
        VulkanCommandPool &operator=(VulkanCommandPool const&) = delete;
        VulkanCommandPool &operator=(VulkanCommandPool &&o) = delete;
        ~VulkanCommandPool() noexcept {
            vkDestroyCommandPool(device, commandPool, nullptr);
        }
    private:
        VkDevice device;
    public:
        VkCommandPool commandPool;
    };
}
#endif // !HG_VULKANCOMMANDPOOL_H
