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
            VkExtent2D const swapChainExtent,
            VkDescriptorSetLayout const &descriptorSetLayout
        ) : device(device)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
            pipelineLayoutInfo.pushConstantRangeCount = 0;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::exception("failed to create pipeline layout!");
            }
        }
        VulkanPipelineLayout(VulkanPipelineLayout const&) = delete;
        VulkanPipelineLayout(VulkanPipelineLayout &&o)
            : device(o.device)
            , pipelineLayout(std::exchange(o.pipelineLayout, VkPipelineLayout{VK_NULL_HANDLE}))
        {
        }
        VulkanPipelineLayout &operator=(VulkanPipelineLayout const&) = delete;
        VulkanPipelineLayout &operator=(VulkanPipelineLayout &&o) {
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
