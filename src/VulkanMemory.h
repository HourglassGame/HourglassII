#ifndef HG_VULKAN_MEMORY_H
#define HG_VULKAN_MEMORY_H
#include <vulkan/vulkan.h>
#include <vector>
namespace hg {
    class VulkanMemory final {
    public:
        explicit VulkanMemory(
            VkDevice const device
        ) : device(device)
          , memory(VK_NULL_HANDLE)
        {}
        explicit VulkanMemory(
            VkDevice const device,
            VkMemoryAllocateInfo const &allocInfo
        ) : device(device)
        {
            if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
                throw std::exception("failed to allocate vertex buffer memory!");
            }
        }
        VulkanMemory(VulkanMemory const&) = delete;
        VulkanMemory(VulkanMemory &&o) noexcept
            : device(o.device)
            , memory(std::exchange(o.memory, VkDeviceMemory{ VK_NULL_HANDLE }))
        {
        }
        VulkanMemory &operator=(VulkanMemory const&) = delete;
        VulkanMemory &operator=(VulkanMemory &&o) noexcept {
            std::swap(device, o.device);
            std::swap(memory, o.memory);
            return *this;
        }
        ~VulkanMemory() noexcept {
            vkFreeMemory(device, memory, nullptr);
        }
        VkDevice device;
        VkDeviceMemory memory;
    };
}
#endif // !HG_VULKAN_MEMORY_H
