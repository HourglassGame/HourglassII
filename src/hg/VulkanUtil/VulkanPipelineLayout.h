#ifndef HG_VULKANPIPELINELAYOUT_H
#define HG_VULKANPIPELINELAYOUT_H
#include <vulkan/vulkan.h>
#include "VulkanShaderModule.h"
namespace hg {
    class VulkanPipelineLayout final {
    public:
        VulkanPipelineLayout(
            VkDevice const device) : device(device), pipelineLayout(VK_NULL_HANDLE)
        {}
        VulkanPipelineLayout(
            VkDevice const device,
            VkPipelineLayoutCreateInfo const &pipelineLayoutInfo
        ) : device(device)
        {
            {
                auto const res{vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create pipeline layout!"));
                }
            }
        }
        VulkanPipelineLayout(VulkanPipelineLayout const&) = delete;
        VulkanPipelineLayout(VulkanPipelineLayout &&o) noexcept
            : device(o.device)
            , pipelineLayout(std::exchange(o.pipelineLayout, VkPipelineLayout{VK_NULL_HANDLE}))
        {
        }
        VulkanPipelineLayout &operator=(VulkanPipelineLayout const&) = delete;
        VulkanPipelineLayout &operator=(VulkanPipelineLayout &&o) noexcept {
            std::swap(device, o.device);
            std::swap(pipelineLayout, o.pipelineLayout);
            return *this;
        }
        ~VulkanPipelineLayout() noexcept {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        }
        VkDevice device;
        VkPipelineLayout pipelineLayout;
    };
}
#endif // !HG_VULKANPIPELINELAYOUT_H
