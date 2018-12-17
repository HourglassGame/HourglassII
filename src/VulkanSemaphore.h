#ifndef HG_VULKANSEMAPHORE_H
#define HG_VULKANSEMAPHORE_H
#include <vulkan/vulkan.h>
#include <vector>
namespace hg {
    class VulkanSemaphore final {
    public:
        explicit VulkanSemaphore(
            VkDevice const device
        ) : device(device)
        {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
                throw std::exception("failed to create semaphore!");
            }
        }
        VulkanSemaphore(VulkanSemaphore const&) = delete;
        VulkanSemaphore(VulkanSemaphore &&o) noexcept
            : device(o.device)
            , semaphore(std::exchange(o.semaphore, VkSemaphore{ VK_NULL_HANDLE }))
        {
        }
        VulkanSemaphore &operator=(VulkanSemaphore const&) = delete;
        VulkanSemaphore &operator=(VulkanSemaphore &&o) noexcept {
            std::swap(device, o.device);
            std::swap(semaphore, o.semaphore);
            return *this;
        }
        ~VulkanSemaphore() noexcept {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        VkDevice device;
        VkSemaphore semaphore;
    };
}
#endif // !HG_VULKANSEMAPHORE_H
