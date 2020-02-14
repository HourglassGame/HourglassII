#ifndef HG_VULKAN_DESCRIPTOR_SET_LAYOUT_H
#define HG_VULKAN_DESCRIPTOR_SET_LAYOUT_H
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanDescriptorSetLayout final {
    public:
        explicit VulkanDescriptorSetLayout(
            VkDevice const device
        ) : device(device), descriptorSetLayout(VK_NULL_HANDLE)
        {}
        explicit VulkanDescriptorSetLayout(
            VkDevice const device,
            VkDescriptorSetLayoutCreateInfo const &layoutInfo
        ) : device(device)
        {
            {
                auto const res{vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create descriptor set layout!"));
                }
            }
        }
        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout const&) = delete;
        VulkanDescriptorSetLayout(VulkanDescriptorSetLayout &&o) noexcept
            : device(o.device)
            , descriptorSetLayout(std::exchange(o.descriptorSetLayout, VkDescriptorSetLayout{ VK_NULL_HANDLE }))
        {
        }
        VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout const&) = delete;
        VulkanDescriptorSetLayout &operator=(VulkanDescriptorSetLayout &&o) noexcept {
            std::swap(device, o.device);
            std::swap(descriptorSetLayout, o.descriptorSetLayout);
            return *this;
        }
        ~VulkanDescriptorSetLayout() noexcept {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        }
        VkDevice device;
        VkDescriptorSetLayout descriptorSetLayout;
    };
}
#endif // !HG_VULKAN_DESCRIPTOR_SET_LAYOUT_H
