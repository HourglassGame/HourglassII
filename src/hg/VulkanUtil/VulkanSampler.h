#ifndef HG_VULKAN_SAMPLER_H
#define HG_VULKAN_SAMPLER_H
#include "VulkanExceptions.h"
#include <boost/throw_exception.hpp>
#include <vulkan/vulkan.h>
#include <system_error>
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
            {
                auto const res{vkCreateSampler(device, &createInfo, nullptr, &sampler)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create image view!"));
                }
            }
        }
        VulkanSampler(VulkanSampler const&) = delete;
        VulkanSampler(VulkanSampler &&o) noexcept
            : device(o.device)
            , sampler(std::exchange(o.sampler, VkSampler{VK_NULL_HANDLE}))
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
