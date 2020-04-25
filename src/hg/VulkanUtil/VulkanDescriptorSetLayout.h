#ifndef HG_VULKAN_DESCRIPTOR_SET_LAYOUT_H
#define HG_VULKAN_DESCRIPTOR_SET_LAYOUT_H
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
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
          , descriptorSetLayout([&]{
                VkDescriptorSetLayout dsl{VK_NULL_HANDLE};
                {
                    auto const res{vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &dsl)};
                    if (res != VK_SUCCESS) {
                        BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create descriptor set layout!"));
                    }
                }
                return dsl;
            }())
        {
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
    private:
        VkDevice device;
    public:
        VkDescriptorSetLayout descriptorSetLayout;
    };
}
#endif // !HG_VULKAN_DESCRIPTOR_SET_LAYOUT_H
