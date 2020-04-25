#ifndef HG_VULKANFRAMEBUFFERHG_H
#define HG_VULKANFRAMEBUFFERHG_H

#include "hg/VulkanUtil/VulkanFramebuffer.h"
#include <vulkan/vulkan.h>
#include <array>

namespace hg {
    class VulkanFramebufferHG final {
    public:
        explicit VulkanFramebufferHG(
            VkDevice const device,
            VkImageView const imageView,
            VkRenderPass const renderPass,
            VkExtent2D const swapChainExtent
        ) : framebuffer(
                device,
                [&](auto const &attachments) {
                    VkFramebufferCreateInfo framebufferInfo = {};
                    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    framebufferInfo.renderPass = renderPass;
                    framebufferInfo.attachmentCount = attachments.size();
                    framebufferInfo.pAttachments = attachments.data();
                    framebufferInfo.width = swapChainExtent.width;
                    framebufferInfo.height = swapChainExtent.height;
                    framebufferInfo.layers = 1;
                    return framebufferInfo;
                }(
                    [&]{
                        return std::array{
                            imageView
                        };
                    }()
                )
            )
        {
        }
        VulkanFramebufferHG(VulkanFramebufferHG const&) = delete;
        VulkanFramebufferHG(VulkanFramebufferHG &&o) noexcept = default;
        VulkanFramebufferHG &operator=(VulkanFramebufferHG const&) = delete;
        VulkanFramebufferHG &operator=(VulkanFramebufferHG &&o) noexcept = default;
        VkFramebuffer h() const noexcept {
            return framebuffer.framebuffer;
        }
    private:
        VulkanFramebuffer framebuffer;
    };
}
#endif // !HG_VULKANFRAMEBUFFER_H
