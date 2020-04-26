#ifndef HG_VULKANRENDERPASSHG_H
#define HG_VULKANRENDERPASSHG_H

#include "hg/VulkanUtil/VulkanRenderPass.h"
#include <vulkan/vulkan.h>
#include <array>

namespace hg {
    class VulkanRenderPassHG final {
    public:
        explicit VulkanRenderPassHG(VkDevice const device)
            : renderPass(device)
        {}
        explicit VulkanRenderPassHG(
            VkDevice const device,
            VkFormat const swapChainImageFormat
        ) : renderPass(
                device,
                [&](auto const &attachmentDescriptions, auto const &subpasses) {
                    VkRenderPassCreateInfo renderPassInfo = {};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                    renderPassInfo.attachmentCount = gsl::narrow<uint32_t>(attachmentDescriptions.size());
                    renderPassInfo.pAttachments = attachmentDescriptions.data();
                    renderPassInfo.subpassCount = gsl::narrow<uint32_t>(subpasses.size());
                    renderPassInfo.pSubpasses = subpasses.data();

                    /*
                    //To satisfy requirements for vkCmdPipelineBarrier in
                    //VulkanRenderTarget
                    std::array<VkSubpassDependency, 1> dependencies{
                        VkSubpassDependency{
                            0,//srcSubpass
                            0,//dstSubpass
                            VK_PIPELINE_STAGE_TRANSFER_BIT,//srcStageMask
                            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,//dstStageMask
                            VK_ACCESS_TRANSFER_WRITE_BIT,//srcAccessMask
                            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,//dstAccessMask
                            0//dependencyFlags
                        }
                    };

                    renderPassInfo.dependencyCount = dependencies.size();
                    renderPassInfo.pDependencies = dependencies.data();
                    */
                    return renderPassInfo;
                }(
                    [&]{
                        VkAttachmentDescription colorAttachment = {};
                        colorAttachment.format = swapChainImageFormat;
                        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                        return std::array{colorAttachment};
                    }(),


                    [](auto const &colorAttachmentRefs){
                        VkSubpassDescription subpass = {};
                        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        subpass.colorAttachmentCount = gsl::narrow<uint32_t>(colorAttachmentRefs.size());
                        subpass.pColorAttachments = colorAttachmentRefs.data();

                        return std::array{subpass};
                    }(
                      []{
                          VkAttachmentReference colorAttachmentRef = {};
                          colorAttachmentRef.attachment = 0;
                          colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                          return std::array{colorAttachmentRef};
                      }())
                )
            )
        {
        }
        VulkanRenderPassHG(VulkanRenderPassHG const&) = delete;
        VulkanRenderPassHG(VulkanRenderPassHG &&o) noexcept = default;
        VulkanRenderPassHG &operator=(VulkanRenderPassHG const&) = delete;
        VulkanRenderPassHG &operator=(VulkanRenderPassHG &&o) noexcept  = default;

        VkRenderPass h() const {
            return renderPass.renderPass;
        }
    private:
        VulkanRenderPass renderPass;
    };
}
#endif // !HG_VULKANRENDERPASSHG_H
