#ifndef HG_VULKANCOMMANDPOOL_H
#define HG_VULKANCOMMANDPOOL_H
#include <vulkan/vulkan.h>
#include "VulkanUtil.h"
#include <utility>
namespace hg {
    class VulkanCommandPool final {
    public:
        VulkanCommandPool(
            VkDevice const device,
            VkPhysicalDevice const physicalDevice,
            VkSurfaceKHR const surface
        ) : device(device)
        {
            QueueFamilyIndices const queueFamilyIndices{findQueueFamilies(physicalDevice, surface)};
            if (!queueFamilyIndices.graphicsFamily) {
                throw std::exception("Couldn't find graphics queue when creating command pool, physical device shouldn't have been selected");
            }
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = *queueFamilyIndices.graphicsFamily;

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw std::exception("failed to create command pool!");
            }
        }
        VulkanCommandPool(VulkanCommandPool const&) = delete;
        VulkanCommandPool(VulkanCommandPool &&o) = delete;
        VulkanCommandPool &operator=(VulkanCommandPool const&) = delete;
        VulkanCommandPool &operator=(VulkanCommandPool &&o) = delete;
        ~VulkanCommandPool() noexcept {
            vkDestroyCommandPool(device, commandPool, nullptr);
        }
        VkDevice device;
        VkCommandPool commandPool;
    };
}
#endif // !HG_VULKANCOMMANDPOOL_H
