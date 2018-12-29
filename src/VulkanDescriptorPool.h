#ifndef HG_VULKAN_DESCRIPTOR_POOL_H
#define HG_VULKAN_DESCRIPTOR_POOL_H
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanDescriptorPool final {
    public:
        explicit VulkanDescriptorPool(
            VkDevice const device
        ) : device(device), descriptorPool(VK_NULL_HANDLE)
        {}
        explicit VulkanDescriptorPool(
            VkDevice         const device,
            VkDescriptorPoolCreateInfo const &createInfo
        ) : device(device)
        {
            if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                throw std::exception("vkCreateDescriptorPool failed!");
            }
        }
        VulkanDescriptorPool(VulkanDescriptorPool const&) = delete;
        VulkanDescriptorPool(VulkanDescriptorPool &&o) noexcept
            : device(o.device)
            , descriptorPool(std::exchange(o.descriptorPool, VkDescriptorPool{VK_NULL_HANDLE}))
        {
        }
        VulkanDescriptorPool &operator=(VulkanDescriptorPool const&) = delete;
        VulkanDescriptorPool &operator=(VulkanDescriptorPool &&o) noexcept {
            std::swap(device, o.device);
            std::swap(descriptorPool, o.descriptorPool);
            return *this;
        }
        ~VulkanDescriptorPool() noexcept {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        }
        VkDevice device;
        VkDescriptorPool descriptorPool;
    };
}
#endif // !HG_VULKAN_DESCRIPTOR_POOL_H