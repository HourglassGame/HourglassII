#ifndef HG_VULKANRENDERPASS_H
#define HG_VULKANRENDERPASS_H
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanRenderPass final {
    public:
        VulkanRenderPass(
            VkDevice const device,
            VkFormat const swapChainImageFormat
        ) : device(device)
        {
            VkAttachmentDescription colorAttachment = {};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;

            if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw std::exception("failed to create render pass!");
            }
        }
        VulkanRenderPass(VulkanRenderPass const&) = delete;
        VulkanRenderPass(VulkanRenderPass &&) = delete;
        VulkanRenderPass &operator=(VulkanRenderPass const&) = delete;
        VulkanRenderPass &operator=(VulkanRenderPass &&) = delete;
        ~VulkanRenderPass() noexcept {
            vkDestroyRenderPass(device, renderPass, nullptr);
        }
        VkDevice device;
        VkRenderPass renderPass;
    };
}
#endif // !HG_VULKANRENDERPASS_H
