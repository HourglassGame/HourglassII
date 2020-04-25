#ifndef HG_VULKANCOMMANDPOOLHG_H
#define HG_VULKANCOMMANDPOOLHG_H
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
            VkPhysicalDevice const physicalDevice,
            VkSurfaceKHR const surface
        ) :
            commandPool(
                device,
                [&]{
                    QueueFamilyIndices const queueFamilyIndices{findQueueFamilies(physicalDevice, surface)};
                    if (!queueFamilyIndices.graphicsFamily) {
                        BOOST_THROW_EXCEPTION(std::exception("Couldn't find graphics queue when creating command pool, physical device shouldn't have been selected"));
                    }

                    VkCommandPoolCreateInfo poolInfo{};
                    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    poolInfo.queueFamilyIndex = *queueFamilyIndices.graphicsFamily;
                    return poolInfo;
                }()
            )
        {
        }
        VulkanCommandPoolHG(VulkanCommandPoolHG const&) = delete;
        VulkanCommandPoolHG(VulkanCommandPoolHG &&o) noexcept = delete;
        VulkanCommandPoolHG &operator=(VulkanCommandPoolHG const&) = delete;
        VulkanCommandPoolHG &operator=(VulkanCommandPoolHG &&o) noexcept = delete;

        VkCommandPool const &h() const noexcept {
            return commandPool.commandPool;
        }
    private:
        VulkanCommandPool commandPool;
    };
}
#endif // !HG_VULKANCOMMANDPOOLHG_H
