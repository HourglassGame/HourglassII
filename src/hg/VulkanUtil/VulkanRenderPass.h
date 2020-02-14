#ifndef HG_VULKANRENDERPASS_H
#define HG_VULKANRENDERPASS_H
#include <vulkan/vulkan.h>
namespace hg {
    class VulkanRenderPass final {
    public:
        explicit VulkanRenderPass(VkDevice const device)
            : device(device)
            , renderPass(VK_NULL_HANDLE)
        {}
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
            {
                auto const res{ vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) };
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create render pass!"));
                }
            }
        }
        VulkanRenderPass(VulkanRenderPass const&) = delete;
        VulkanRenderPass(VulkanRenderPass &&o) noexcept
            : device(o.device)
            , renderPass(std::exchange(o.renderPass, VkRenderPass{VK_NULL_HANDLE}))
        {}
        VulkanRenderPass &operator=(VulkanRenderPass const&) = delete;
        VulkanRenderPass &operator=(VulkanRenderPass &&o) noexcept {
            std::swap(device, o.device);
            std::swap(renderPass, o.renderPass);
            return *this;
        }
        ~VulkanRenderPass() noexcept {
            vkDestroyRenderPass(device, renderPass, nullptr);
        }
        VkDevice device;
        VkRenderPass renderPass;
    };
}
#endif // !HG_VULKANRENDERPASS_H
