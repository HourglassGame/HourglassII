#ifndef HG_VULKANFRAMEBUFFER_H
#define HG_VULKANFRAMEBUFFER_H

#include "VulkanExceptions.h"
#include <vulkan/vulkan.h>
#include <utility>
namespace hg {
    class VulkanFramebuffer final {
    public:
        VulkanFramebuffer(
            VkDevice const device,
            VkImageView const imageView,
            VkRenderPass const renderPass,
            VkExtent2D const swapChainExtent
        ) : device(device)
        {
            VkImageView attachments[] = {
                imageView
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            {
                auto const res{ vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer)};
                if (res != VK_SUCCESS) {
                    BOOST_THROW_EXCEPTION(std::system_error(res, "failed to create framebuffer!"));
                }
            }
        }
        VulkanFramebuffer(VulkanFramebuffer const&) = delete;
        VulkanFramebuffer(VulkanFramebuffer &&o) noexcept
            : device(o.device)
            , framebuffer(std::exchange(o.framebuffer, VkFramebuffer{ VK_NULL_HANDLE }))
        {
        }
        VulkanFramebuffer &operator=(VulkanFramebuffer const&) = delete;
        VulkanFramebuffer &operator=(VulkanFramebuffer &&o) noexcept {
            std::swap(device, o.device);
            std::swap(framebuffer, o.framebuffer);
            return *this;
        }
        ~VulkanFramebuffer() noexcept {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        VkDevice device;
        VkFramebuffer framebuffer;
    };
}
#endif // !HG_VULKANFRAMEBUFFER_H
