#ifndef HG_VULKAN_BUFFER_H
#define HG_VULKAN_BUFFER_H
#include <vulkan/vulkan.h>
#include <vector>
namespace hg {
    class VulkanBuffer final {
    public:
        explicit VulkanBuffer(VkDevice const device)
            : device(device), buffer(VK_NULL_HANDLE)
        {}
        explicit VulkanBuffer(
            VkDevice const device,
            VkBufferCreateInfo const &bufferInfo
        ) : device(device)
        {
            if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
                throw std::exception("failed to create vertex buffer!");
            }
        }
        VulkanBuffer(VulkanBuffer const&) = delete;
        VulkanBuffer(VulkanBuffer &&o) noexcept
            : device(o.device)
            , buffer(std::exchange(o.buffer, VkBuffer{ VK_NULL_HANDLE }))
        {
        }
        VulkanBuffer &operator=(VulkanBuffer const&) = delete;
        VulkanBuffer &operator=(VulkanBuffer &&o) noexcept {
            std::swap(device, o.device);
            std::swap(buffer, o.buffer);
            return *this;
        }
        ~VulkanBuffer() noexcept {
            vkDestroyBuffer(device, buffer, nullptr);
        }
        VkDevice device;
        VkBuffer buffer;
    };
}
#endif // !HG_VULKAN_BUFFER_H
