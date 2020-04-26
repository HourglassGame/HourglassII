#ifndef HG_VULKANSEMAPHOREHG_H
#define HG_VULKANSEMAPHOREHG_H
#include "hg/VulkanUtil/VulkanSemaphore.h"
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanSemaphoreHG final {
    public:
        explicit VulkanSemaphoreHG(
            VkDevice const device
        ) : semaphore(
                device,
                []{
                    VkSemaphoreCreateInfo semaphoreInfo = {};
                    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                    return semaphoreInfo;
                }()
            )
        {
        }
        VulkanSemaphoreHG(VulkanSemaphoreHG const&) = delete;
        VulkanSemaphoreHG(VulkanSemaphoreHG &&o) noexcept = default;
        VulkanSemaphoreHG &operator=(VulkanSemaphoreHG const&) = delete;
        VulkanSemaphoreHG &operator=(VulkanSemaphoreHG &&o) noexcept = default;

        VkSemaphore h() const {
            return semaphore.semaphore;
        }
    private:
        VulkanSemaphore semaphore;
    };
}
#endif // !HG_VULKANSEMAPHOREHG_H
