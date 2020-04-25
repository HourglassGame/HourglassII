#ifndef HG_VULKAN_DESCRIPTOR_POOL_H
#define HG_VULKAN_DESCRIPTOR_POOL_H
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
namespace hg {
    class VulkanDescriptorPool final {
    public:
        explicit VulkanDescriptorPool(
            VkDevice const device
        ) : device(device), descriptorPool(VK_NULL_HANDLE)
        {}

        explicit VulkanDescriptorPool(
            VkDevice const device,
            VkDescriptorPoolCreateInfo const &createInfo
        ) : device(device)
          , descriptorPool([&]{
                VkDescriptorPool dp{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateDescriptorPool(device, &createInfo, nullptr, &dp)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "vkCreateDescriptorPool failed!"));
                    }
                }
                return dp;
            }())
        {
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
    private:
        VkDevice device;
    public:
        VkDescriptorPool descriptorPool;
    };
}
#endif // !HG_VULKAN_DESCRIPTOR_POOL_H
