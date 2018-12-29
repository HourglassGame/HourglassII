#ifndef HG_VULKANPIPELINELAYOUT_H
#define HG_VULKANPIPELINELAYOUT_H
#include <vulkan/vulkan.h>
#include "VulkanShaderModule.h"
namespace hg {
    class VulkanPipelineLayout final {
    public:
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
        VulkanPipelineLayout(VulkanPipelineLayout &&) = delete;
        VulkanPipelineLayout &operator=(VulkanPipelineLayout const&) = delete;
        VulkanPipelineLayout &operator=(VulkanPipelineLayout &&) = delete;
        ~VulkanPipelineLayout() noexcept {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        }
        VkDevice device;
        VkPipelineLayout pipelineLayout;
    };
}
#endif // !HG_VULKANPIPELINELAYOUT_H
