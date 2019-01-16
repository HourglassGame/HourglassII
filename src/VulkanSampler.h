#ifndef HG_VULKAN_SAMPLER_H
#define HG_VULKAN_SAMPLER_H
#include <vulkan/vulkan.h>
#include <utility>
namespace hg {
    class VulkanSampler final {
    public:
        explicit VulkanSampler(
            VkDevice const device
        ) : device(device)
          , sampler(VK_NULL_HANDLE)
        {}
        explicit VulkanSampler(
            VkDevice const device,
            VkSamplerCreateInfo const &createInfo
        ) : device(device)
        {
            if (vkCreateSampler(device, &createInfo, nullptr, &sampler) != VK_SUCCESS) {
                throw std::exception("failed to create image view!");
            }
        }
        VulkanSampler(VulkanSampler const&) = delete;
        VulkanSampler(VulkanSampler &&o) noexcept
            : device(o.device)
            , sampler(std::exchange(o.sampler, VkSampler{ VK_NULL_HANDLE }))
        {
        }
        VulkanSampler &operator=(VulkanSampler const&) = delete;
        VulkanSampler &operator=(VulkanSampler &&o) noexcept {
            std::swap(device, o.device);
            std::swap(sampler, o.sampler);
            return *this;
        }
        ~VulkanSampler() noexcept {
            vkDestroySampler(device, sampler, nullptr);
        }
        VkDevice device;
        VkSampler sampler;
    };
}
#endif // !HG_VULKAN_SAMPLER_H
