#ifndef HG_VULKANFENCEHG_H
#define HG_VULKANFENCEHG_H
#include "hg/VulkanUtil/VulkanFence.h"
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanFenceHG final {
    public:
        explicit VulkanFenceHG(
            VkDevice const device
        ) : fence(
                device,
                []{
                    VkFenceCreateInfo fenceInfo{};
                    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                    return fenceInfo;
                }()
            )
        {
        }
        VulkanFenceHG(VulkanFenceHG const&) = delete;
        VulkanFenceHG(VulkanFenceHG &&o) = default;
        VulkanFenceHG &operator=(VulkanFenceHG const&) = delete;
        VulkanFenceHG &operator=(VulkanFenceHG &&o) = default;
        VkFence const &h() const noexcept {
            return fence.fence;
        }
    private:
        VulkanFence fence;
    };
}
#endif // !HG_VULKANFENCEHG_H
