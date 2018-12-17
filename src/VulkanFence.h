#ifndef HG_VULKANFENCE_H
#define HG_VULKANFENCE_H
#include <vulkan/vulkan.h>
#include <vector>
namespace hg {
    class VulkanFence final {
    public:
        explicit VulkanFence(
            VkDevice const device
        ) : device(device)
        {
            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
                throw std::exception("failed to create fence!");
            }
        }
        VulkanFence(VulkanFence const&) = delete;
        VulkanFence(VulkanFence &&o) noexcept
            : device(o.device)
            , fence(std::exchange(o.fence, VkFence{ VK_NULL_HANDLE }))
        {
        }
        VulkanFence &operator=(VulkanFence const&) = delete;
        VulkanFence &operator=(VulkanFence &&o) noexcept {
            std::swap(device, o.device);
            std::swap(fence, o.fence);
            return *this;
        }
        ~VulkanFence() noexcept {
            vkDestroyFence(device, fence, nullptr);
        }
        VkDevice device;
        VkFence fence;
    };
}
#endif // !HG_VULKANFENCE_H
