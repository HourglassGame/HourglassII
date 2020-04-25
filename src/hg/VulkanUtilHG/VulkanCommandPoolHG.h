#ifndef HG_VULKANCOMMANDPOOLHG_H
#define HG_VULKANCOMMANDPOOLHG_H
#include "VulkanUtilPhysicalDevice.h"

#include "hg/VulkanUtil/VulkanCommandPool.h"
#include "hg/VulkanUtil/VulkanUtil.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
namespace hg {
    class VulkanCommandPoolHG final {
    public:
        explicit VulkanCommandPoolHG(
            VkDevice const device,
            PossiblePhysicalDevice const &physicalDevice,
            VkSurfaceKHR const surface
        ) :
            commandPool(
                device,
                [&]{
                    VkCommandPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    poolInfo.queueFamilyIndex = physicalDevice.queueIndices.graphicsFamily;
                    return poolInfo;
                }()
            )
        {
        }
        VulkanCommandPoolHG(VulkanCommandPoolHG const&) = delete;
        VulkanCommandPoolHG(VulkanCommandPoolHG &&o) noexcept = delete;
        VulkanCommandPoolHG &operator=(VulkanCommandPoolHG const&) = delete;
        VulkanCommandPoolHG &operator=(VulkanCommandPoolHG &&o) noexcept = delete;

        VkCommandPool h() const noexcept {
            return commandPool.commandPool;
        }
    private:
        VulkanCommandPool commandPool;
    };
}
#endif // !HG_VULKANCOMMANDPOOLHG_H
